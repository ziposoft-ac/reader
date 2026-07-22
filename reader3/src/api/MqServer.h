//
// Created by ac on 7/14/26.
//

#ifndef ZIPOSOFT_IPCSERVER_H
#define ZIPOSOFT_IPCSERVER_H
#include "pch.h"
#include "MqClient.h"

#include "api/CommandHandler.h"


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

    virtual void thread() ;

    std::set<CommandHandler*> _cmdHandlers;

    std::thread _thread_handle;
protected:
    z_string _q_name="/mq_server";

public:
    bool is_running() {  return _is_running;  }
    void register_consumer(CommandHandler* consumer);
    void remove_consumer(CommandHandler* consumer);
    MqServer();
    ~MqServer();
    //z_safe_queue<RfidRead*> _queue_reads;
    virtual z_status run(ctext name,int size=0x2000);
    z_status shutdown();

    virtual  z_status process_command_handlers(MqMsg* msg);
    virtual  int process_message(MqMsg* msg) {
        return 0;

    }
    z_status send_msg(ctext mq_name,ctext command,ctext data,size_t len);
    z_status send_msg_self(mq_command_enum_t command);

    z_status start();
    z_status stop();
    z_status send(z_string mq_name,z_string msg);

};
template <class T> class MqServerCb : public MqServer {
public:

    T* _object=0;
    typedef int (T::*member_callback)(MqMsg* msg) ;
    member_callback _member_callback=0;
    virtual z_status run(ctext name,T* obj,member_callback callback) {
        _object=obj;
        _member_callback=callback;
        _q_name=name;
        return zs_ok;
    };
    int process_message(MqMsg* msg) override {
        if (_member_callback)
            return  (_object->*_member_callback)(msg);

        return 0;

    }
};

template <class T> class MqServerMap : public MqServer {

public:
    typedef z_status (T::*member_callback_t)(const char* data);
    T* _object=0;
    typedef struct  {
        ctext name;
        member_callback_t callback;
        size_t data_size;
    } entry_t;
    static size_t map_size;
    static entry_t  map[];

    z_status run_map(ctext queue_name,T* obj) {
        _object=obj;
        return run(queue_name);
    }
    virtual  int process_message(MqMsg* msg) {
        entry_t *entry=0;
        for (int i=0;i<map_size;i++) {
            if (strcmp(map[i].name,msg->command_str) == 0) {
                entry=&map[i];
                break;
            }

        }
        if (!entry) {

            return Z_ERROR_MSG(zs_bad_command,"MQ command not found: %s\n",msg->command_str);
        }

        if (entry->data_size != msg->data_len) {
            return Z_ERROR_MSG(zs_bad_command,"MQ data size for command does not match: %s\n",msg->command_str);
        }
        member_callback_t callback=entry->callback;

        return  (_object->*callback)(msg->data);

    }

};

ZMETA_DECL(MqServer) {
    ZACT(stop);
    ZACT(start);
    ZSTAT(is_running);
    ZPROP(_q_name);
    ZCMD(send, ZFF_CMD_DEF, "send",
        ZPRM(z_string, mq_name, "mq", "mq_name", ZFF_PARAM),
         ZPRM(z_string, msg, "hello there", "msg", ZFF_PARAM)

         );
}

class MqServerTest : public MqServer{
    friend z_factory_t<MqServerTest>;

private:

public:

    virtual  int process_message(MqMsg* msg) {
        printf("MqServerTest RX: %s\n",msg->command_str);
        return 0;
    }


};






#endif //ZIPOSOFT_IPCSERVER_H
