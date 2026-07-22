//
// Created by ac on 7/18/26.
//

#ifndef ZIPOSOFT_LEDAPI_H
#define ZIPOSOFT_LEDAPI_H



#include "pch.h"

#include "zipolib/z_static_map.h"

enum LedColor {
    LedRed=1,
    LedGreen=2,
    LedYellow=3
};
enum LedOperation {
    LedBlink,
    LedOn,
    LedOff
};
struct LedCommand {
    LedColor color;
    LedOperation operation;

};
struct LedSet
{
    LedColor color;
    bool on;

};
struct LedFlash
{
    LedColor color;
    U32 time_ms;
    U32 count;
    U32 dummy;
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
#include "api/ApiDeclare.inc"






#endif //ZIPOSOFT_LEDAPI_H
