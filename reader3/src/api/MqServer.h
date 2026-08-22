//
// Created by ac on 7/14/26.
//

#ifndef ZIPOSOFT_IPCSERVER_H
#define ZIPOSOFT_IPCSERVER_H
#include "pch.h"
#include "MqClient.h"
#include "util/timers.h"

class CommandHandler;

// get rid of this
template <class OBJ> class MqApiHandler {
public:

    typedef z_status (OBJ::*member_callback)(void* msg);
    template <typename T> using member_callback_t = z_status (OBJ::*)(T* data);


    z_stl_map<z_string,member_callback> _callbacks;
    OBJ* _object=0;
    MqApiHandler(OBJ* obj) {
        _object=obj;
    }

    template <typename DATA_T> void add_callback(ctext name, z_status (OBJ::*callback)(DATA_T*)) {
        _callbacks[name]=(member_callback)callback;

    }
    z_status exec_callback(ctext name,void* data) {
        member_callback callback;
        if (!_callbacks.get(name,callback)) {
            return Z_ERROR_MSG(zs_bad_command,"MQ command not found: %s\n",name);
        }
        return  (_object->*callback)(data);


    }

};





class MqServer {
    friend z_factory_t<MqServer>;

private:
    U32 _send_msg_id=1;
    mqd_t _mq_server_fd=0;
    bool _is_running=false;
    bool _shutdown_flag=false;
    virtual void thread() ;

    std::set<CommandHandler*> _cmdHandlers;

    std::thread _thread_handle;
protected:

public:
    z_string _q_name="/mq_server";

    bool _debug=false;
    bool is_running() {  return _is_running;  }
    void register_consumer(CommandHandler* consumer);
    void remove_consumer(CommandHandler* consumer);
    MqServer();
    ~MqServer();
    //z_safe_queue<RfidRead*> _queue_reads;
    virtual z_status run(ctext name);
    virtual z_status shutdown();

    virtual  z_status process_command_handlers(MqMsg* msg);
    virtual  z_status process_message(MqMsg* msg) {
        return Z_ERROR_MSG(zs_not_implemented,"MQ msg not processed\n");

    }
    //z_status send_msg(ctext mq_name,ctext command,ctext data,size_t len);
    z_status send_msg_self(mq_command_enum_t command);

    z_status start();
    z_status stop();
    z_status send(z_string mq_name,z_string msg);

};
class MqFeed;
constexpr uint feed_keep_alive_ms=5*1000;
class FeedSubscriber {
public:

    z_string _name;
    z_string _rx_q_name;
    MqFeed* _feed;
    z_time _last_msg_time=0;
    FeedSubscriber(MqFeed* feed,ctext name);

    z_status send(mq_command_enum_t cmd_type,ctext command,mq_data_type_t data_type,z_string* buffer=0);



};

class MqFeed : public MqServer {
    std::mutex _lock;
    z_obj_map<FeedSubscriber> _subscribers;
    friend z_factory_t<FeedSubscriber>;
    Timer* _timer=0;

public:
    friend z_factory_t<MqFeed>;
    int timer_callback(void*) {
        z_string s;
        ZDBG("Sending keep alive\n");
        publish("ping",s,mq_command_keep_alive);
        return feed_keep_alive_ms;
    }
    z_status sendToSub(ctext subname,mq_command_enum_t cmd_type,ctext command,mq_data_type_t data_type,z_string* buffer=0);

    z_status remove_all_subscribers();
    z_status delete_mqueue_nodes();
    U32 num_subscribers() {
        return _subscribers.size();
    }
    virtual z_status shutdown() override;

    virtual z_status run(ctext name) override;

    z_status publish(ctext command,z_string& buffer,mq_command_enum_t cmd_type=mq_command_string);
    z_status publish(ctext command);

    virtual z_status process_message(MqMsg* msg) override;



};

template <class T> class MqServerCb : public MqServer {
public:

    T* _object=0;
    typedef z_status (T::*member_callback)(MqMsg* msg) ;
    member_callback _member_callback=0;
    virtual z_status run(ctext name,T* obj,member_callback callback) {
        _object=obj;
        _member_callback=callback;
        return MqServer::run(name);
    };
    z_status process_message(MqMsg* msg) override {
        if (_member_callback)
            return  (_object->*_member_callback)(msg);

        return zs_ok;

    }
};


ZMETA_DECL(MqServer) {
    ZACT(stop);
    ZACT(start);
    ZSTAT(is_running);
    ZPROP(_q_name);
    ZPROP(_debug);
    ZCMD(send, ZFF_CMD_DEF, "send",
        ZPRM(z_string, mq_name, "mq", "mq_name", ZFF_PARAM),
         ZPRM(z_string, msg, "hello there", "msg", ZFF_PARAM)

         );
}

class MqServerTest : public MqServer{
    friend z_factory_t<MqServerTest>;

private:

public:

    virtual  z_status process_message(MqMsg* msg) {
        printf("MqServerTest RX: %s\n",msg->command_str);
        return zs_ok;
    }


};






#endif //ZIPOSOFT_IPCSERVER_H
