//
// Created by ac on 9/19/25.
//

#ifndef ZIPOSOFT_MONGOOSE_H
#define ZIPOSOFT_MONGOOSE_H
#include "pch.h"

#include "mongoose/mongoose.h"

enum cmd_req_type
{
    REQUEST_INVALID,
    REQUEST_POST,
    REQUEST_GET,
};
struct http_request
{
    mg_connection *c;
    struct mg_http_message *hm;
    int index;


};

typedef void (*fn_cmd_reply_t) (z_json_stream &js,size_t ctx1,size_t ctx2);
typedef int (*fn_cmd_post_t) (http_request req,z_json_obj &jin);
typedef int (*fn_cmd_get_t) (http_request req,z_string_map &vars);
typedef int (*fn_cmd_t) (http_request req);
void send_headers(struct mg_connection *c);

template<typename T> int send_json_response(http_request r,T callback) {
    z_string s;
    z_string msg_out;
    z_json_stream js(s);
    js.obj_start();

    callback(js);

    js.obj_end();

    send_headers(r.c);

    mg_http_write_chunk(r.c, s.c_str(), s.length());

    mg_http_printf_chunk(r.c, ""); // Don't forget the last empty chunk
    return 0;
}

struct cmd_entry_t
{
    ctext cmd_name;
    cmd_req_type type;
    //fn_cmd_get_t fn;
    size_t fn;
    //fn_cmd_get_t fn;
};

struct delayed_request {
    http_request r;
    cmd_entry_t* cmd;
    U64 ts_expire;
    size_t ctx1;
    size_t ctx2;
    fn_cmd_reply_t fn_complete;
};
#endif //ZIPOSOFT_MONGOOSE_H