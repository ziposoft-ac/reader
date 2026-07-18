//
// Created by ac on 11/13/20.
//

#ifndef ZIPOSOFT_ROOT_H
#define ZIPOSOFT_ROOT_H


#include "pch.h"
#include "io/BeepPwm.h"
#include "api/MqServer.h"

#include "rfid/cfmu804.h"
#include "util/timers.h"
#include "tests/tests.h"
#include "rfid/simulator.h"
#include "io/gpioButton.h"
#include "battery/Battery.h"
#include "io/gpio.h"
#include "io/i2c.h"
#include "web/WebServer.h"
#include "rfid/VisitProcess.h"
#include "rfid/ReaderService.h"
#include "leds/LedService.h"
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
    I2c i2c;
    Battery battery;
    gpioButton button;
    VisitProcess visitProc;
    Tests tests;
    WebServer web_server;
    ReaderService readerService;
    BeepPwm beeper;
    MqServerTest mqServerTest;

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
