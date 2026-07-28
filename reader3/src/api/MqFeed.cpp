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
z_status MqFeed::sendToSub(ctext subname,mq_command_enum_t cmd_type,ctext command,z_string* buffer) {

    z_string rx_q_name=_q_name;
    rx_q_name<<"-"<<subname;
    ZDBG("sending to %s\n",rx_q_name.c_str());

    z_status status=mq_send_msg(rx_q_name,cmd_type,command,buffer);
    return status;

}

z_status MqFeed::publish(ctext command,z_string& buffer) {
    std::unique_lock mlock(_lock);


    auto it = _subscribers.begin();
    while (it != _subscribers.end())
    {
        ctext name=it->first;
        z_status status=sendToSub(name,mq_command_json,command,&buffer);
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
    return zs_ok;

}
z_status MqFeed::remove_all_subscribers() {


    std::unique_lock mlock(_lock);
    _subscribers.delete_all();
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

    publish("close");
    remove_all_subscribers();

    return MqServer::shutdown();

}



z_status MqFeed::run(ctext feedname) {

    _q_name=feedname;
    remove_all_subscribers();


    return MqServer::run(feedname);


}

z_status MqFeed::process_message(MqMsg *msg) {

    if ((msg->command_enum < mq_command_subscribe)||(msg->command_enum > mq_command_unsubscribe))
        return MqServer::process_message(msg);
    z_string sub_name;

    sub_name.assign(msg->data,msg->data_len);


    std::unique_lock mlock(_lock);

    if (msg->command_enum == mq_command_subscribe) {
        ZDBG("got subscribe request from :%s\n",sub_name.c_str());
        FeedSubscriber* sub=new FeedSubscriber(sub_name);
        _subscribers.add(sub_name,sub);
        z_status status=sendToSub(sub_name,mq_command_subscribe_ack,0);
        return status;

    }
    if (msg->command_enum == mq_command_unsubscribe) {
        _subscribers.del(sub_name);
        return zs_ok;

    }
    return Z_ERROR_MSG(zs_not_found,"MqFeed unhandled command \n");

}
