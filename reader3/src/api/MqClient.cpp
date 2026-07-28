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
