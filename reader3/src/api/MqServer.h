//
// Created by ac on 7/14/26.
//

#ifndef ZIPOSOFT_IPCSERVER_H
#define ZIPOSOFT_IPCSERVER_H
#include "pch.h"
#include "IpcRequests.h"


#include <mqueue.h>


#define MAX_MSG_SIZE 0x2000


template<typename DATA_T> struct MqMsg_T {
    U16 mq_reply_name_len;
    U16 command_str_len;
    U16 command_enum;
    U32 data_len;
    U32 msg_id;
    ctext mq_reply_name;
    ctext command_str;
    DATA_T* data;

    ctext buffer;
    U32 buff_len;
};
typedef MqMsg_T<const char> MqMsg;

z_status msg_create(
    MqMsg* msg,
    ctext mq_reply_name,
    ctext command,
    U16  command_enum,
    U32 msg_id,
    U32 data_len,
    ctext data
);
enum mq_command_enum_t {
    mq_command_string,
    mq_command_quit,

};

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


template <class OBJ,typename MSG_TYPE> class MqCallHandler {

    MqCallHandler(OBJ* obj,z_status (OBJ::*callback)(MSG_TYPE* msg)) {

    }

};


/*



*/




class MqServer {
    friend z_factory_t<MqServer>;

private:
    U32 _send_msg_id=1;
    mqd_t _mq_server_fd=0;
    bool is_running=false;

    virtual void thread() ;


    std::thread _thread_handle;
protected:
    z_string _this_mq_server_name="/mq_server";

public:
    MqServer();
    ~MqServer();
    //z_safe_queue<RfidRead*> _queue_reads;
    z_status run(ctext name,int size=0x2000);
    z_status shutdown();


    virtual  int process_message(MqMsg* msg) {
        return 0;

    }
    z_status send_msg(ctext mq_name,ctext command,ctext data,size_t len);
    z_status send_msg_self(mq_command_enum_t command);

    z_status start();
    z_status stop();
    z_status send(z_string mq_name,z_string msg);

};
ZMETA_DECL(MqServer) {
    ZACT(stop);
    ZACT(start);
    ZPROP(_this_mq_server_name);
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
