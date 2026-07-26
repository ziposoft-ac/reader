//
// Created by ac on 7/14/26.
//

#ifndef ZIPOSOFT_MQCLIENT_H
#define ZIPOSOFT_MQCLIENT_H
#include "pch.h"


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

void msg_destroy(MqMsg* msg);


enum mq_command_enum_t {
    mq_command_string,
    mq_command_json=1,
    mq_command_internal_wakeup=2,
    mq_command_ack=3,
    mq_command_subscribe=10,
    mq_command_subscribe_ack=11,
    mq_command_unsubscribe=12,
    mq_command_unsubscribe_ack=13,
    mq_command_close=14,

};


z_status mq_send_msg_with_reply(ctext mq_name_dest,ctext mq_name_reply,mq_command_enum_t cmd_type,ctext command,U32 msg_id,ctext data,size_t data_len);
z_status mq_send_msg(ctext mq_name_dest,mq_command_enum_t cmd_type,ctext command,ctext data="",size_t data_len=0);
z_status mq_send_msg(ctext mq_name_dest,mq_command_enum_t cmd_type,ctext command,z_string* s);
template <typename T> z_status mq_send_msg_t(ctext mq_name_dest,mq_command_enum_t cmd_type,ctext command,const T& data) {
    return mq_send_msg(mq_name_dest,cmd_type,command,(ctext)&data,sizeof(T));


}




#endif //ZIPOSOFT_IPCSERVER_H
