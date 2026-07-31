//
// Created by ac on 7/8/26.
//

#ifndef ZIPOSOFT_BEEPPWM_H
#define ZIPOSOFT_BEEPPWM_H
#include "pch.h"
#include "../util/timers.h"


//typedef std::pair<U16,U16> Tone; // freq,dur
struct Tone {
    U16 freq=0; U16 duration=1;
    Tone(int f, int d) : freq(f), duration(d) {}
    Tone(){}
};
struct Note {
    U16 freq=0; U16 duration=1;U16 duty=0;
    Note(){}
    Note(Tone t,U16 duty) : freq(t.freq), duration(t.duration), duty(duty) {}
    Note(int f, int d,int v) : freq(f), duration(d), duty(v) {}
};

const int RemoteBeepMaxLength=5;
struct RemoteBeep_t
{
    U16 count;
    Note notes[RemoteBeepMaxLength];
};
class BeepPwm {
protected:
    virtual int timer_callback(void*);
    z_safe_queue<Note> _queue;
    Timer* _timer=0;
    bool _initialized=false;
    bool _open=false;

public:
    bool _exists=false;
    bool _quiet=false;
    bool _enabled=false;
    U16 _duty=50;
    BeepPwm() {}

    bool exists() {return _exists;}
    virtual ~BeepPwm(){}
    // a good BLEEP =  1000,50,500,50,1600,50

    z_status buzz(int f0,int d0,int f1,int d1,int f2,int d2);
    z_status beep(int tone,int duration,int volume);
    z_status toneRise();
    z_status takeOnMe();
    z_status takeOnMePush();
    virtual z_status init();
    virtual z_status shutdown();

    z_status pushRemoteBeep(RemoteBeep_t *);
    void pushTones(std::initializer_list<Tone> const beeps);
    void pushNotes(std::initializer_list<Note> const beeps);
};


#endif //ZIPOSOFT_BEEPPWM_H
