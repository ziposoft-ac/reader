//
// Created by ac on 8/2/21.
//

#ifndef ZIPOSOFT_JSONCMD_H
#define ZIPOSOFT_JSONCMD_H
#include "pch.h"
#include "WebRequests.h"




#define CMDS \
    CMD_GET(status) \
    CMD_GET(reads) \
    CMD_GET(config) \
    CMD_GET(gpio) \
    CMD_POST(gpio) \
    CMD_POST(config) \
    CMD_GET(delay) \
    CMD_POST(stop) \
    CMD_POST(start) \
    CMD_POST(stop_app0) \
    CMD_POST(start_app0) \
    CMD_GET(beep) \
    CMD_POST(beepon) \
    CMD_POST(beepoff) \
    CMD_GET(test)



extern cmd_entry_t cmd_list[];
extern const size_t cmd_list_size;

#endif //ZIPOSOFT_JSONCMD_H
