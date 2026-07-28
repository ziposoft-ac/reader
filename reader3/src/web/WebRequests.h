//
// Created by ac on 9/19/25.
//

#ifndef WEBREQ_H
#define WEBREQ_H
#include "pch.h"
#define CONNECTION_CLOSE_ALL 0

typedef struct delayed_request delayed_request;
typedef struct mg_connection mg_connection;
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
    //  int index;


};

void get_params_from_req(http_request r,z_string_map& var_map);
void send_default_headers(struct mg_connection *c,int status);
#endif //ZIPOSOFT_MONGOOSE_H