//
// Created by ac on 7/19/26.
//
#include "MqClient.h"
#include <mqueue.h>

void msg_destroy(MqMsg* msg) {
    if (msg->buffer)
        delete []msg->buffer;
    msg->buffer=NULL;
    msg->buff_len=0;
}

z_status mq_send_msg_with_reply(ctext mq_name_dest,ctext mq_name_reply,ctext command,U32 msg_id,ctext data,size_t data_len) {
    MqMsg msg;

    //ZDBG("sending %s,%s,%s,datlen=%d\n",mq_name_dest,mq_name_reply,command,data_len);
    msg_create(&msg,mq_name_reply,command,mq_command_string,msg_id,data_len,data);

    mqd_t mq_server = mq_open(mq_name_dest, O_WRONLY);
    if (mq_server == (mqd_t)-1) {
        perror("Failed to open Server  queue");
        return zs_could_not_open_file;
    }
    // Send the response back to the specific client
    if (mq_send(mq_server, msg.buffer, msg.buff_len, 0) == -1) {
        perror("Server: mq_send  failed");
    }

    msg_destroy(&msg);
    mq_close(mq_server);
    return zs_success;
}
z_status mq_send_msg(ctext mq_name_dest,ctext command,ctext data,size_t data_len) {

    if (data && (data_len==0))
        data_len=strlen(data);
    return mq_send_msg_with_reply(mq_name_dest,"",command,0,data,data_len);


}
z_status mq_send_msg(ctext mq_name_dest,ctext command,z_string& s) {

    return mq_send_msg_with_reply(mq_name_dest,"",command,0,s.c_str(),s.length());


}
