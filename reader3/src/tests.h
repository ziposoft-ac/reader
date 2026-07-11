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
    int _interval=20;
    //int _time_off=1000;
    //int _state=0;
    int _iterations=10;
    int _current_iteration=0;
    virtual z_status stop();
    virtual z_status start();
    virtual int onCallback(void*)
    {
        return 0;//return next interval value, 0 to stop
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
class TestTimerInterval : public TestTimer{
public:
    int onCallback(void*) override;
    int _last_iter=0;
    int _print_interval_seconds=3;

};
class TestTimerSimple : public TestTimer{
public:
    int onCallback(void*) override {
        printf("iter=%d\n", _current_iteration++);
        if (_current_iteration<_iterations)
            return _interval;
        return 0;
    }
    virtual z_status onStart()
    {
        _current_iteration=0;
        return zs_ok;
    }

};

class TestTimerCascade : public TestTimer{
public:
    int onCallback(void*) override;
    int timer_callback2(void*);
    z_status onStop() override;
    z_status onStart() override;

    Timer* _timer2=0;
};
class TestThread : public Test {
    std::thread _thread_handle;
    void _thread() {
        thread();
    }
protected:
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


/*
class TestWsBlast : public Test{
public:
    int onCallback(void*);
    virtual z_status onStart();

};
*/

class ReaderTest : public TestTimer{
public:
    int _time_off=1000;
    int _time_on=1000;
    virtual z_status onStart();
    virtual z_status onStop();
    virtual int onCallback(void*);
    int _read_pause_time=4;
    int _session=0;

};
class TestHeatTest : public ReaderTest{
public:
    virtual z_status onStart();
    virtual z_status onStop();
    virtual int onCallback(void*);
    int _max_temp_shutoff=50;

};
class Inventory : public TestThread{
public:
    int _target=0;
    int _session=1;
    bool _send_stop=false;
    int _scan_time=0;
    int _backoff=0;
    virtual z_status onStart();
    virtual z_status onStop();
    virtual void thread();

};
class TestGpioOnOff : public TestTimer{
public:
    int _gpioNum=20;
    int _time_off=1000;
    int _time_on=1000;
    int _state=0;
    virtual z_status onStop();
    int onCallback(void*);

};
class TestLedFlash : public TestGpioOnOff{
public:


};

class Tests {
public:
    z_status shutdown();
    z_status test_timestamp();
    TestGpioOnOff gpioOnOff;
    TestLedFlash flashleds;
    TestHeatTest heatTest;
    ReaderTest readTest;
    TestTimerInterval timerInterval;
    TestTimerCascade timerCascade;
    TestTimerSimple timerSimple;
    TestPipe pipe;
    TestThread thread;
    Inventory inv;
    //ReadPipe readPipe;
};

ZMETA_DECL(Test) {
    ZACT(stop);
    ZACT(start);
    ZPROP_F(_running,ZFF_READ_ONLY);
}
ZMETA_DECL(TestTimer) {
    ZBASE(Test);
    ZPROP(_interval);

    ZPROP(_iterations);
}
ZMETA_DECL(TestThread) {
    ZBASE(Test);

}

#endif //ZIPOSOFT_TESTS_H
