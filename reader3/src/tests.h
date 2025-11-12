//
// Created by ac on 11/13/20.
//

#ifndef ZIPOSOFT_TESTS_H
#define ZIPOSOFT_TESTS_H

#include "pch.h"
#include "timers.h"


class Test {
protected:
public:
    bool _running=0;

    virtual z_status stop()    {
        return zs_ok;
    }
    virtual z_status start()    {
        return zs_ok;
    }

    virtual z_status onStop()
    {
        return zs_ok;
    }
    virtual z_status onStart()
    {
        return zs_ok;
    }
};
class TestTimer : public Test {
    int timer_callback(void*);

    Timer* _timer=0;
public:
    int _time_on=20;
    int _time_off=1000;
    int _state=0;
    int _iterations=10;
    int _count=0;
    virtual z_status stop();
    virtual z_status start();
    virtual int onCallback(void*)
    {
        return 0;
    }
    virtual z_status onStop()
    {
        return zs_ok;
    }
    virtual z_status onStart()
    {
        return zs_ok;
    }
};
class TestThread : public Test {
    std::thread _thread_handle;
    void _thread() {
        thread();
    }

    bool _quit=false;
public:
    virtual z_status stop();
    virtual z_status start();
    virtual void thread() {
        printf("test thread running");
    }
};
class ReadPipe : public TestThread {
public:
    virtual void thread() {
        printf("TestThread2 running");
    }
};
class TestPipe {
    int _fPipe=0;
    int _fQuitEvent=0;

public:
    z_status openread();
    z_status openwrite();
    z_status read();
    z_status write(z_string data)
    {
        openwrite();
        ::write(_fPipe, data.c_str(), data.size());
        ::write(_fPipe, "\n\0", 2);
        return zs_ok;
    }

    z_string _name="/tmp/debugview";
};

class TestTimers : public TestTimer{
public:
    int onCallback(void*);

};
/*
class TestWsBlast : public Test{
public:
    int onCallback(void*);
    virtual z_status onStart();

};
*/
class TestHeatTest : public TestTimer{
public:
    virtual z_status onStart();
    virtual z_status onStop();
    int onCallback(void*);
    int _read_pause_time=4;
    int _max_temp_shutoff=50;

};
class TestGpioOnOff : public TestTimer{
public:
    int _gpioNum=20;
    virtual z_status onStop();
    int onCallback(void*);

};
class TestLedFlash : public TestGpioOnOff{
public:


};

class Tests {
public:
    z_status shutdown();
    TestGpioOnOff gpioOnOff;
    TestLedFlash flashleds;
    TestHeatTest heatTest;
    TestTimers timer;
    TestPipe pipe;
    TestThread thread;
    //ReadPipe readPipe;
};

ZMETA_DECL(Test) {
    ZACT(stop);
    ZACT(start);
    ZPROP_F(_running,ZFF_READ_ONLY);
}
ZMETA_DECL(TestTimer) {
    ZBASE(Test);

    ZPROP(_time_on);
    ZPROP(_time_off);
    ZPROP(_iterations);
}
ZMETA_DECL(TestThread) {
    ZBASE(Test);

}

#endif //ZIPOSOFT_TESTS_H
