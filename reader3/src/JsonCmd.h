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
    CMD_POST(config) \
    CMD_GET(delay) \
    CMD_POST(stop) \
    CMD_POST(start) \
    CMD_GET(test)



extern cmd_entry_t cmd_list[];
extern const size_t cmd_list_size;

#endif //ZIPOSOFT_JSONCMD_H
