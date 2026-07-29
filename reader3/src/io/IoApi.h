//
// Created by ac on 7/27/26.
//

#ifndef ZIPOSOFT_IOAPI_H
#define ZIPOSOFT_IOAPI_H
#include "pch.h"

constexpr ctext ioServiceName="/ioservice";

enum LedColor {
    LedRed=1,
    LedGreen=2,
    LedYellow=3,
    LedMax=4
};
struct LedSet_t
{
    LedColor color;
    bool on;

};
struct LedFlash_t
{
    LedColor color;
    U32 time_ms;
    U32 count;
    U32 dummy;
};

z_status ioLedSet(LedSet_t set);
z_status ioLedFlash(LedFlash_t flash);

#endif //ZIPOSOFT_IOAPI_H
