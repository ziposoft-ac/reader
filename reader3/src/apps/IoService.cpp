//
// Created by ac on 7/17/26.
//
#include "pch.h"
#include "main/Service.h"
#include "battery/Battery.h"
#include "io/gpioButton.h"
#include "io/gpio.h"
#include "web/WebServer.h"
#include "io/BeepPwm.h"
#include "api/IoApi.h"

#include "global.h"


struct Counter {
    U64 count;

};




class IoService : public  Service,public CommandHandler{
public:
    //BeepPwm beeper;
    IoApiTest apiTest;
    WebServer ws;
    MqServer mq;
    Battery bat;
    GpioButton button;
    IoService(){}
    virtual ~IoService() {}
    z_status handleSetLed(LedSet_t* set) {

        GpioPinLed* leds[]={0,
            &gGpio.ledRed,&gGpio.ledGreen,&gGpio.ledYellow
        };
        GpioPinLed* led=0;
        U8 color=set->color;
        if (color<LedMax)
            led=leds[color];
        if (led) {
            (set->on? led->on():led->off());
            return zs_ok;

        }
        return zs_bad_parameter;

    }

    z_status handleBeep(RemoteBeep_t* set) {


        gBeepPwm.pushRemoteBeep(set);
        return zs_ok;
    }
    z_status handleFlashLed(LedFlash_t* set) {
        printf("handleFlashLed:%d,%d\n",set->color,set->count);

        return zs_ok;
    }

    int get_status_json(z_string_map& params,z_json_stream& jout) {
        bat.json_get(jout);
        return 0;
    }
    int get_beep_json(z_string_map& params,z_json_stream& jout) {
        int freq=params.get_as<int>("freq",500);
        int len=params.get_as<int>("len",50);
        int duty=params.get_as<int>("duty",10);
        gBeepPwm.beep(freq,len,duty);
        return 0;
    }
    z_status initialize() override{
        //ZDBGS.add_stdout();
        gGpio.initialize();
        gBeepPwm.init();
        bat.init();


        reg_bin_func("setLed",&IoService::handleSetLed);
        reg_func("stat",&IoService::get_status_json);
        reg_func("",&IoService::get_status_json);
        reg_func("beep",&IoService::get_beep_json);
        reg_bin_func("beepMq",&IoService::handleBeep);
        ws.register_consumer(this);
        mq.register_consumer(this);
        bat.start();

        ws._port=8001;
        ws.start();
        mq.run(ioServiceName);
        button.start();
        return zs_ok;
    };
    z_status shutdown() override{
        button.stop();
        ws.stop();
        gBeepPwm.shutdown();
        mq.shutdown();
        gGpio.shutdown();
        bat.shutdown();

        return zs_ok;
    };
    U64 _counter=0;

};
ZMETA(IoService) {
    ZBASE(Service);
    ZOBJ(apiTest);
    ZOBJ(button);
    ZOBJ(mq);
    ZOBJ(ws);
    ZOBJ(bat);
    ZOBJ_EX(gGpio,"gpio",ZFF_PROP_DEF,"global gpio object");
    ZOBJ_EX(gBeepPwm,"beeper",ZFF_PROP_DEF,"global beep PWM");
};

ROOT_SERVICE(IoService);
