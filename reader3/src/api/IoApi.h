//
// Created by ac on 7/27/26.
//

#ifndef ZIPOSOFT_IOAPI_H
#define ZIPOSOFT_IOAPI_H
#include "pch.h"
#include "io/BeepPwm.h"
#include "api/MqClient.h"
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
    LedColor color=LedGreen;
    U32 time_ms=100;
    U32 count=1;
};

z_status ioBeep(U16 duty,std::initializer_list<Tone> const tones);
z_status ioLedSet(LedSet_t set);
z_status ioLedFlash(LedFlash_t flash);



class IoApiTest {
    public:
    z_status testBeep() {
        return ioBeep(2,{{500,20},{0,20},{500,20}});
    }
    z_status setLed(int color, int onoff) {
        ioLedSet({(LedColor)color,(bool)onoff});

        return zs_ok;
    }
    z_status ledFlash(int color, int onoff) {
        LedFlash_t f;
        ioLedFlash(f);

        return zs_ok;
    }


};
#endif //ZIPOSOFT_IOAPI_H
