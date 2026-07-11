//
// Created by ac on 11/13/20.
//

#ifndef ZIPOSOFT_ROOT_H
#define ZIPOSOFT_ROOT_H


#include "pch.h"
#include "BeepPwm.h"

#include "cfmu804.h"
#include "timers.h"
#include "tests.h"
#include "simulator.h"
#include "gpioButton.h"
#include "gpio.h"
#include "WebServer.h"
#include "VisitProcess.h"
#include "ReaderService.h"
//#include "processRunner.h"

class Root
{
    friend z_factory_t<Root>;

    std::condition_variable _cv_quit;
    std::mutex _mutex_quit;
    bool _shutting_down=false;
    bool _auto_start_server=false;
    bool _auto_start_app=false;
    bool _init_rfid=true;
    bool _simulate=true;
    RfidReader* _reader;
    bool _enable_logging=false;
public:
    Root();
    virtual ~Root();
    z_console console;
    Cfmu804 cfmu804;
    RfidSimulator simulator;
    // server;
    Gpio gpio;
    gpioButton button;
    VisitProcess visitProc;
    Tests tests;
    WebServer web_server;
    TimerService timerService;
    ReaderService readerService;
    BeepPwm beeper;

    //ProcessRunner processRunner;

    int _test_count=0;
    void wait_for_quit()
    {
        std::unique_lock<std::mutex> mlock(_mutex_quit);

        _cv_quit.wait(mlock);
    }
    bool shuttingDown() {return _shutting_down;}

    z_status shutdown();
    z_status initialize();

    z_status quit_notify();

    RfidReader& getReader() { return *_reader; }

    z_status dump_ports();

    z_status simulate_on() {
        _reader=&simulator;
        _simulate=true;
        return zs_ok;
    }
    z_status simulate_off() {
        _reader=&cfmu804;
        _simulate=false;
        return zs_ok;
    }

};

extern Root root;



#endif //ZIPOSOFT_ROOT_H
