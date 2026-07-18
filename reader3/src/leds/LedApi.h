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

};

struct ApiEntry {
    size_t dataSize;
    ctext name;

};

#define LED_API \
    API(LedService,LedService, \
    CMD(LedFlash)    \
    )







constexpr auto MyStaticMap = make_static_map<std::string_view, int>({
    {"apple",  1},
    {"banana", 2},
    {"cherry", 3}
});






#endif //ZIPOSOFT_LEDAPI_H
