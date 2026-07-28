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
    LedYellow=3
};
struct LedSet_t
{
    LedColor color;
    U8 on;

};
struct LedFlash_t
{
    LedColor color;
    U32 time_ms;
    U32 count;
    U32 dummy;
};


#endif //ZIPOSOFT_IOAPI_H
