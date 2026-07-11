//
// Created by ac on 7/8/26.
//

#ifndef ZIPOSOFT_BEEPPWM_H
#define ZIPOSOFT_BEEPPWM_H
#include "pch.h"
#include "timers.h"
typedef std::pair<int,int> Tone;

class BeepPwm {
protected:
    virtual int _off();
    virtual int _on();
    virtual int timer_callback(void*);
    z_safe_queue<Tone> _queue;
    Timer* _timer=0;
    bool _initialized=false;
    bool _open=false;

public:
    bool _exists=false;
    bool _quiet=false;
    bool _enabled=false;
    int _duty=50;
    BeepPwm() {}

    bool exists() {return _exists;}
    virtual ~BeepPwm(){}
    // a good BLEEP =  1000,50,500,50,1600,50

    z_status buzz(int f0,int d0,int f1,int d1,int f2,int d2);
    z_status toneRise();
    z_status takeOnMe();
    z_status takeOnMePush();
    virtual z_status init();
    virtual void shutdown();
    void pushBeeps(std::initializer_list<Tone> const beeps);
};


#endif //ZIPOSOFT_BEEPPWM_H
