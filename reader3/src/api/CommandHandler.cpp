#include "CommandHandler.h"
#include "mongoose/mongoose.h"
#include "zipolib/http_status.h"

/*
ZMETA(CommandDelayed) {
    ZACT(complete);


};*/

int Command::call_post_raw_data(ctext buffer,size_t len,z_string &return_buffer) {
    zp_text_parser p;
    z_json_obj json_in=p.makeJsonObj(buffer,len);
    z_string msg_out;
    z_json_stream json_out(return_buffer);
    json_out.obj_start();
    json_out.keyval_int("ts",z_time::get_now_ms());
    //post_json_reply(json_in,json_out);
    int ret=  0;//  callback_http(req,var_map,jin,json_out);
    json_out.obj_end();
    return ret;


}


int Command::process_mq(MqMsg* msg) {
    z_string return_buffer;
    http_status_t http_status=HTTP_STATUS_OK;

    z_string msg_out;

    int ret=call_post_raw_data(msg->data,msg->data_len,msg_out);

    if (ret!=0) {
        return ret;
    }
    if (msg->mq_reply_name && msg->mq_reply_name_len > 1) {
        z_string reply="@";
        reply+=msg->command_str;
        mq_send_msg_with_reply(msg->mq_reply_name,"",mq_command_ack,reply,msg->msg_id,return_buffer.c_str(),return_buffer.length());
    }
    return ret;


}


int Command::process_http_rx(http_request req,cmd_req_type type) {

    z_string return_buffer;
    http_status_t http_status=HTTP_STATUS_OK;
    int callback_ret=-1;
    z_json_stream json_out(return_buffer);
    json_out.obj_start();
    z_json_obj jin;
    z_json_obj jout;
    z_string_map var_map;
        get_params_from_req(req,var_map);

    if (type==REQUEST_GET) {
        //post_json_reply(json_in,json_out);
    }
    else {
        zp_text_parser p;

        p.parseJsonObj(jin,req.hm->body.buf,req.hm->body.len);

    }
    callback_ret=callback_http(req,var_map,jin,json_out);

    json_out.obj_end();
    if (callback_ret==HTTP_STATUS_PROCESSING)
        return HTTP_STATUS_PROCESSING;
    if (callback_ret==0) {
        http_status=HTTP_STATUS_OK;
    }
    else {
        http_status=HTTP_STATUS_SERVICE_UNAVAILABLE;
    }
    send_default_headers(req.c,http_status);

    mg_http_write_chunk(req.c, return_buffer.c_str(), return_buffer.length());

    mg_http_printf_chunk(req.c, ""); // Don't forget the last empty chunk
    return http_status;




}


int CommandHandler::process_http_close(ulong id) {
    std::unique_lock mlock(_map_mutex);
    for (auto i : _map) {
        i.second->process_http_close(id);
    }

    return 0;
}

