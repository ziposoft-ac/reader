//
// Created by ac on 8/2/21.
//

#ifndef ZIPOSOFT_JSONCMD_H
#define ZIPOSOFT_JSONCMD_H
#include "pch.h"
#include "WebRequests.h"
#include "zipolib/http_status.h"


enum delayed_request_type {
    DELAYED_REQUEST_READS_RAW,
    DELAYED_REQUEST_READS_FILTERED,
    DELAYED_REQUEST_TEST
};

#define CMDS \
    CMD_GET(status) \
    CMD_GET(reads_raw) \
    CMD_GET(reads_filtered) \
    CMD_GET(visits) \
    CMD_GET(config) \
    CMD_GET(gpio) \
    CMD_POST(gpio) \
    CMD_POST(program_bcd) \
    CMD_POST(program_epc) \
    CMD_GET(read_one) \
    CMD_POST(config) \
    CMD_GET(delay) \
    CMD_POST(stop_raw) \
    CMD_POST(start_raw) \
    CMD_POST(stop_visitProc) \
    CMD_POST(start_visitProc) \
    CMD_GET(beep) \
    CMD_POST(beepon) \
    CMD_POST(beepoff) \
    CMD_GET(pingpong)



extern cmd_entry_t cmd_list[];
extern const size_t cmd_list_size;
void send_command_response(http_request r,z_status zstatus,http_status_t http_status,const z_string& msg);
#endif //ZIPOSOFT_JSONCMD_H
