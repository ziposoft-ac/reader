//
// Created by ac on 7/19/26.
//
#include "MqClient.h"
#include <mqueue.h>


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

void msg_destroy(MqMsg* msg) {
    if (msg->buffer)
        delete []msg->buffer;
    msg->buffer=NULL;
    msg->buff_len=0;
}

z_status mq_send_msg_with_reply(ctext mq_name_dest,ctext mq_name_reply,mq_command_enum_t cmd_type,ctext command,U32 msg_id,
    mq_data_type_t data_type,ctext data,size_t data_len) {
    MqMsg msg;
    z_status status=zs_fatal_error;
    ZDBG("sending %s,%s,%s,datlen=%d\n",mq_name_dest,mq_name_reply,command,data_len);
    msg_create(&msg,mq_name_reply,(command?command:""),cmd_type,msg_id,data_type,data_len,data);

    mqd_t mq_server = mq_open(mq_name_dest, O_WRONLY|O_NONBLOCK);
    if (mq_server == (mqd_t)-1) {

        return Z_ERROR_MSG(zs_could_not_open_file,"Could not open mq:%s\n",mq_name_dest);
    }
    // Send the response back to the specific client
    int ret=mq_send(mq_server, msg.buffer, msg.buff_len, 0);

    if (ret==0) {
        ZDBG("sent ok\n");
        status=zs_ok;

    }
    else
    {
        if (errno == EAGAIN) {
            status=zs_device_busy;
            ZDBG("queue is full\n");

            // The queue is full.
            // Implement your fallback here (e.g., retry later, log, or drop).
        } else {
            perror("error sending mq\n");
            // A different error occurred (e.g., EBADF, EINVAL).
        }
    }


    msg_destroy(&msg);
    mq_close(mq_server);

    return status;
}
z_status mq_send_msg(ctext mq_name_dest,mq_command_enum_t cmd_type,ctext command,mq_data_type_t data_type,ctext data,size_t data_len) {

    if (data && (data_len==0))
        data_len=strlen(data);
    return mq_send_msg_with_reply(mq_name_dest,"",cmd_type,command,0,data_type,data,data_len);


}
z_status mq_send_msg(ctext mq_name_dest,mq_command_enum_t cmd_type,ctext command,mq_data_type_t data_type,z_string* s) {


    return mq_send_msg_with_reply(mq_name_dest,"",cmd_type,command,0,data_type,(s?s->c_str():0),(s?s->length():0));


}
