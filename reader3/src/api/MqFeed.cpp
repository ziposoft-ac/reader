//
// Created by ac on 7/24/26.
//
#include <filesystem>
#include "MqServer.h"
namespace fs = std::filesystem;
ZMETA(MqFeed) {
    ZBASE(MqServer);

};
z_status MqFeed::publish(ctext command) {
    z_string s;

    return publish(command,s);

}

FeedSubscriber::FeedSubscriber(MqFeed *feed, ctext name): _name(name) {
    _rx_q_name=name;
    _feed=feed;
}

z_status FeedSubscriber::send(mq_command_enum_t cmd_type, ctext command, mq_data_type_t data_type, z_string *buffer) {




    z_status status=mq_send_msg(_rx_q_name,cmd_type,command,data_type,buffer);


    return status;
}



z_status MqFeed::sendToSub(ctext subname,mq_command_enum_t cmd_type,ctext command,mq_data_type_t data_type,
    z_string* buffer) {

    ZDBG("sending to %s\n",subname);

    z_status status=mq_send_msg(subname,cmd_type,command,data_type,buffer);
    return status;

}

z_status MqFeed::publish(ctext command,z_string& buffer,mq_command_enum_t cmd_type) {
    std::unique_lock mlock(_lock);


    auto it = _subscribers.begin();
    while (it != _subscribers.end())
    {
        ctext name=it->first;
        z_status status=sendToSub(name,cmd_type,command,mq_data_json,&buffer);
        if (status==zs_ok) {
            it++;
        }
        else
        {
            ZDBG("error sending, removing %s\n",name);
            FeedSubscriber* sub=it->second;
            delete sub;
            it=_subscribers.erase(it);
        }
    }
    if (_timer)
        _timer->start_ms_reset(feed_keep_alive_ms);

    return zs_ok;

}
z_status MqFeed::remove_all_subscribers() {


    std::unique_lock mlock(_lock);
    _subscribers.delete_all();

    return zs_ok;

}
z_status MqFeed::delete_mqueue_nodes() {


    std::string mqueue_path = "/dev/mqueue";

    if (!fs::exists(mqueue_path)) {
        std::cerr << "Error: /dev/mqueue does not exist. Is the mqueue filesystem mounted?\n";
        return zs_fatal_error;
    }

    for (const auto& entry : fs::directory_iterator(mqueue_path)) {
        if (entry.is_regular_file()) {

            std::cout << entry.path().filename().string() << '\n';

            std::string subname= entry.path().filename().string();
            subname="/"+subname;

            if (subname.starts_with(_q_name.c_str())) {

                ZDBG("deleting client queue: %s\n",subname.c_str());
                z_status status=mq_send_msg(subname.c_str(),mq_command_feed_close,"close");

                //mq_unlink(subname.c_str());

            }




        }
    }
    return zs_ok;

}

z_status MqFeed::shutdown() {
    if (_timer) {
        _timer->stop();

    }
    publish("close");
    remove_all_subscribers();

    return MqServer::shutdown();

}



z_status MqFeed::run(ctext feedname) {

    _q_name=feedname;
    remove_all_subscribers();

    if(!_timer) {
        _timer=CREATE_TIMER(MqFeed::timer_callback );
    }
    _timer->start_ms_reset(feed_keep_alive_ms);
    return MqServer::run(feedname);


}

z_status MqFeed::process_message(MqMsg *msg) {

    if ((msg->command_enum < mq_command_subscribe)||(msg->command_enum > mq_command_unsubscribe))
        return MqServer::process_message(msg);
    z_string sub_name;

    sub_name=msg->mq_reply_name;


    std::unique_lock mlock(_lock);

    if (msg->command_enum == mq_command_subscribe) {
        ZDBG("got subscribe request from :%s\n",sub_name.c_str());
        if (_subscribers.exists(sub_name)) {
            ZDBG("subscriber already exists :%s\n",sub_name.c_str());

        }
        else {
            ZDBG("adding subscriber :%s\n",sub_name.c_str());

            FeedSubscriber* sub=new FeedSubscriber(this,sub_name);
            _subscribers.add(sub_name,sub);
        }

        z_status status=sendToSub(sub_name,mq_command_subscribe_ack,"",mq_data_none);
        return status;

    }
    if (msg->command_enum == mq_command_unsubscribe) {
        _subscribers.del(sub_name);
        return zs_ok;

    }
    return Z_ERROR_MSG(zs_not_found,"MqFeed unhandled command \n");

}
