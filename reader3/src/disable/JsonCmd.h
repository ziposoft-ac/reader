//
// Created by ac on 8/2/21.
//

#ifndef ZIPOSOFT_JSONCMD_H
#define ZIPOSOFT_JSONCMD_H
#include "pch.h"
#include "../web/WebRequests.h"
#include "zipolib/http_status.h"


enum delayed_request_type {
    DELAYED_REQUEST_READS_RAW,
    DELAYED_REQUEST_READS_FILTERED,
    DELAYED_REQUEST_TEST
};

#define CMDS \
    CMD_POST(test500) \
    CMD_POST(pong) \
    CMD_GET(delay) \
    CMD_GET(pingpong)



extern cmd_entry_t cmd_list[];
extern const size_t cmd_list_size;
void send_command_response(http_request r,z_status zstatus,http_status_t http_status,const z_string& msg);
#endif //ZIPOSOFT_JSONCMD_H
