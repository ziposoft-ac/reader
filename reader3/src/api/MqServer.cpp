//
// Created by ac on 7/14/26.
//

#include "MqServer.h"
#include "CommandHandler.h"
#include <mqueue.h>
ZMETA_DEF(MqServer);
ZMETA(MqServerTest) {
    ZBASE(MqServer);

};

#define	MQ_DBG(...) {if(_debug) ZDBG(__VA_ARGS__); }

MqServer::MqServer() {}

MqServer::~MqServer() {

}

z_status msg_create(MqMsg* msg,ctext mq_reply_name, ctext command,mq_command_enum_t  command_enum,
    U32 msg_id,mq_data_type_t data_type, U32 data_len, ctext data) {
    size_t len_cmd= strlen(command)+1;
    size_t len_name= strlen(mq_reply_name)+1;
    size_t len_total=len_cmd+len_name+data_len+
        sizeof(U16)+ // mq_reply_name_len
        sizeof(U16)+ // command_str_len
        sizeof(U8)+ // command_enum
        sizeof(U8)+ // data_type
        sizeof(U32)+ // data_len
        sizeof(U32); // msg_id

    if (data==nullptr)
        data="";

    if (len_total >= MAX_MSG_SIZE) {
        return Z_ERROR_MSG(zs_bad_parameter,"total msg exceeds max len: %s",command);
    }
    msg->buff_len=len_total;
    ctext b=msg->buffer=new char[len_total];
    *(U16*)b=len_name;b+=sizeof(U16);
    *(U16*)b=len_cmd;b+=sizeof(U16);
    *(U8*)b=command_enum;b+=sizeof(U8);
    *(U8*)b=data_type;b+=sizeof(U8);
    *(U32*)b=data_len;b+=sizeof(U32);
    *(U32*)b=msg_id;;b+=sizeof(U32);
    memcpy((void*)b,mq_reply_name,len_name);b+=len_name;
    memcpy((void*)b,command,len_cmd);;b+=len_cmd;
    memcpy((void*)b,data,data_len);
    return zs_ok;
}






z_status msg_deserialize(MqMsg* msg,ctext buff,size_t len) {
    ctext p=buff;
    ctext end=buff+len;
    msg->mq_reply_name_len=*(U16*)p;p+=sizeof(U16);
    if (p>end) return zs_parse_error;



    msg->command_str_len=*(U16*)p;p+=sizeof(U16);
    if (p>end) return zs_parse_error;

    msg->command_enum=*(U8*)p;p+=sizeof(U8);
    if (p>end) return zs_parse_error;
    msg->data_type=*(U8*)p;p+=sizeof(U8);
    if (p>end) return zs_parse_error;
    msg->data_len=*(U32*)p;p+=sizeof(U32);
    if (p>end) return zs_parse_error;

    msg->msg_id=*(U32*)p;p+=sizeof(U32);
    if (p>end) return zs_parse_error;

    msg->mq_reply_name=p;p+=msg->mq_reply_name_len;
    if (p>end) return zs_parse_error;

    //check null terminator
    if (0 != *(p-1))
        return zs_parse_error;

    msg->command_str=p;p+=msg->command_str_len;
    if (p>end) return zs_parse_error;
    //check null terminator
    if (0 != *(p-1))
        return zs_parse_error;
    msg->data=p;
    p+=msg->data_len;
    if (p>end) return zs_parse_error;


    return zs_ok;


}
z_status MqServer::process_command_handlers(MqMsg* msg) {

    for(auto handler :_cmdHandlers ) {
        Command* cmd=handler->get_command(msg->command_str);
        if (cmd) {

            int ret= cmd->process_mq(msg);

            return zs_ok;;

        }
    }

    return zs_not_found;

}



void MqServer::thread() {
    // Block until a client sends a message
    unsigned int msg_priority=0;
    char* buff=new char[MAX_MSG_SIZE+1];
    MqMsg msg;
    _is_running=true;
    while (1) {
        ssize_t len=mq_receive(_mq_server_fd, buff, MAX_MSG_SIZE, &msg_priority);
        if (len == -1) {
            perror("Server: mq_receive failed");
            break;
        }
        z_status status=msg_deserialize(&msg,buff,len);
        if (status) {
            Z_ERROR_LOG("Error parsing RX MQ msg");
            continue;
        }

        MQ_DBG("MQ RX command %s\n",msg.command_str);
        if (_shutdown_flag)
            break;
        if (msg.command_enum == mq_command_internal_wakeup) {

            // ignore these
            continue;
        }

        if (process_command_handlers(&msg)==zs_not_found) {
                process_message(&msg);

        }


    }
    delete []buff;
    printf("Mq server quiting thread\n");
    _is_running=false;


}

z_status MqServer::shutdown() {

    if (_is_running) {
        _shutdown_flag=true;
        send_msg_self(mq_command_internal_wakeup);
        if (_thread_handle.joinable())
            _thread_handle.join();

    }
    if (!_mq_server_fd)
        return zs_not_open;

    mq_close(_mq_server_fd);
    printf("Mq server shutdown\n");
    _mq_server_fd=0;
    return zs_ok;
}
z_status MqServer::send_msg_self(mq_command_enum_t command_enum) {
    MqMsg msg;

    msg_create(&msg,"","",command_enum,_send_msg_id,mq_data_none,0,0);
    _send_msg_id++;
    if (mq_send(_mq_server_fd, msg.buffer, msg.buff_len, 10000) == -1) {
        perror("Client: mq_send failed");
    }
    msg_destroy(&msg);
    return zs_success;
}

z_status MqServer::start() {
    return run(_q_name);
}

z_status MqServer::run(ctext name) {

    if (_mq_server_fd)
        return zs_already_open;
    struct mq_attr attr;
    mq_unlink(name);
    // Configure queue parameters
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 0x2000;
    attr.mq_curmsgs = 0;
    ZDBG("opening mq server:%s\n",name);
    _q_name=name;

    _mq_server_fd = mq_open(name, O_RDWR | O_CREAT, 0660, &attr);
    if (_mq_server_fd == (mqd_t)-1) {
        Z_ERROR_LOG("Unable to open:%s\n",name);
        perror("Server: mq_open failed");
        _mq_server_fd=0;
        return zs_could_not_open_file;
    }
    _thread_handle = std::thread(&MqServer::thread, this);


    return zs_success;
}


z_status MqServer::send(z_string remote_mq_name, z_string msg) {


    return send_msg(remote_mq_name,msg,0,0);

}



z_status MqServer::stop() {
    shutdown();
    return zs_success;
}
void MqServer::register_consumer(CommandHandler *consumer) {

    if(_cmdHandlers.find(consumer)==_cmdHandlers.end()) {

        _cmdHandlers.insert(consumer);
        //ZLOG("Registered consumer for RfidReader\n");
    }
}

void MqServer::remove_consumer(CommandHandler *consumer) {
    if(_cmdHandlers.find(consumer)!=_cmdHandlers.end())
        _cmdHandlers.erase(consumer);
}
