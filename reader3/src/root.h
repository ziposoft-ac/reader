//
// Created by ac on 11/13/20.
//

#ifndef ZIPOSOFT_ROOT_H
#define ZIPOSOFT_ROOT_H


#include "pch.h"

#include "cfmu804.h"
#include "timers.h"
#include "tests.h"
#include "simulator.h"
#include "gpio.h"
#include "WebServer.h"
#include "app0.h"
//#include "processRunner.h"

class Root
{
    friend z_factory_t<Root>;

    std::condition_variable _cv_quit;
    std::mutex _mutex_quit;
    bool _shutting_down=false;

public:
    Root();
    virtual ~Root();
    z_console console;
    Cfmu804 cfmu804;
    RfidSimulator simulator;
    // server;
    Gpio gpio;
    App0 app0;
    Tests tests;
    WebServer web_server;
    TimerService timerService;
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


    z_status dump_ports();

    z_status run_app();
    z_status run_as_service();


};

extern Root root;



#endif //ZIPOSOFT_ROOT_H
