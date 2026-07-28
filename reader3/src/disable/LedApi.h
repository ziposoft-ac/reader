//
// Created by ac on 7/18/26.
//

#ifndef ZIPOSOFT_LEDAPI_H
#define ZIPOSOFT_LEDAPI_H




#include "zipolib/z_static_map.h"


enum LedOperation {
    LedBlink,
    LedOn,
    LedOff
};
struct LedCommand {
    LedColor color;
    LedOperation operation;

};

struct Dummy
{
    U32 dummy;

};


#define LED_API \
    API_NAME(LedService, \
    CMD(LedFlash)    \
    CMD(LedSet)    \
    CMD(Dummy)    \
    )

#define API LED_API
#include "ApiDeclare.inc"






#endif //ZIPOSOFT_LEDAPI_H
