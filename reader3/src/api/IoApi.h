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
    U32 on;

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



class IoApiClient {
    public:
    z_status testBeep() {
        return ioBeep(2,{{500,20},{0,20},{500,20}});
    }
    z_status setLed(int color, int onoff) {
        ioLedSet({(LedColor)color,(U32)onoff});

        return zs_ok;
    }
    z_status ledFlash(int color, int onoff) {
        LedFlash_t f;
        ioLedFlash(f);

        return zs_ok;
    }
    z_status buzz(int duty,int f0,int d0,int f1,int d1,int f2,int d2);


};

ZMETA_DECL(IoApiClient) {
    ZACT(testBeep);
    ZCMD(setLed, ZFF_CMD_DEF, "setLed",
         ZPRM(int, color, 1, "color", ZFF_PARAM),
         ZPRM(int, on, 1, "onoff", ZFF_PARAM)
    );

    ZCMD(ledFlash, ZFF_CMD_DEF, "ledFlash",
         ZPRM(int, color, 1, "color", ZFF_PARAM),
         ZPRM(int, on, 1, "onoff", ZFF_PARAM)
    );

    ZCMD(buzz, ZFF_CMD_DEF, "buzz",
         ZPRM(int, duty, 2, "duty", ZFF_PARAM),
         ZPRM(int, f0, 1000, "freq0", ZFF_PARAM),
         ZPRM(int, d0, 20, "duration0", ZFF_PARAM),
         ZPRM(int, f1, 0, "freq1", ZFF_PARAM),
         ZPRM(int, d1, 0, "duration1", ZFF_PARAM),
         ZPRM(int, f2, 0, "freq2", ZFF_PARAM),
         ZPRM(int, d2, 0, "duration2", ZFF_PARAM)
    );
};
#endif //ZIPOSOFT_IOAPI_H
