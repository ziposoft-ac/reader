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
    IoApiClient apiTest;
    WebServer ws;
    MqServer mq;
    Battery bat;
    GpioButton button;
    IoService(){}
    virtual ~IoService() {}
    std::vector<GpioPinLed*> _leds;


    int _battery_poll_interval=5000;
    int _beep_enable=false;
    int _beep_max_volume=1;
    int _max_beep_queue=10;
    int post_config(z_json_obj& o,z_json_stream& jout) {
        _battery_poll_interval=o.get_int("batteryPollInterval",_battery_poll_interval);
        _beep_max_volume=o.get_int("maxVolume",_beep_max_volume);
        _max_beep_queue=o.get_int("maxBeepQueue",_max_beep_queue);
        _beep_enable=o.get_bool("beepEnable",false);

        // Ugly dont store in 2 places
        gBeepPwm._max_beep_queue=_max_beep_queue;
        gBeepPwm._max_duty=_beep_max_volume;
        gBeepPwm._output_enable=_beep_enable;
        status_json(jout);
#ifdef NOGPIO
        o.print();
#endif

        return 0;

    }
    z_status add_json_config(z_json_stream &js) {

        js.key("io_config");

        js.obj_start();
        js.key_bool("beepEnable",_beep_enable);
        js.keyval_int("batteryPollInterval",_battery_poll_interval);
        js.keyval_int("maxVolume",_beep_max_volume);
        js.keyval_int("maxBeepQueue",_max_beep_queue);
        js.obj_end();
        return zs_ok;
    }

    z_status handleSetLed(LedSet_t* set) {


        GpioPinLed* led=0;
        U8 color=set->color;
        if (color<LedMax)
            led=_leds[color];
        if (led) {
            (set->on? led->on():led->off());
            return zs_ok;

        }
        return zs_bad_parameter;

    }
    int takeOnMe(z_string_map& params,z_json_stream& jout) {


        gBeepPwm.takeOnMePush();
        return zs_ok;
    }
    z_status handleBeep(RemoteBeep_t* set) {


        gBeepPwm.pushRemoteBeep(set);
        return zs_ok;
    }
    z_status handleFlashLed(LedFlash_t* set) {

        GpioPinLed* led=0;
        U8 color=set->color;
        if (color<LedMax)
            led=_leds[color];
        if (led) {
            led->flash(set->count);
            return zs_ok;

        }
        return zs_bad_parameter;
        return zs_ok;
    }
    int status_json(z_json_stream& jout) {
        bat.json_get(jout);
        add_json_config(jout);
        return 0;
    }
    int get_status_json(z_string_map& params,z_json_stream& jout) {
        return status_json(jout);
    }
    int get_beep_json(z_string_map& params,z_json_stream& jout) {
        int freq=params.get_as<int>("freq",500);
        int len=params.get_as<int>("len",50);
        int duty=params.get_as<int>("duty",10);
        gBeepPwm.beep(freq,len,duty);
        return status_json(jout);
    }


    z_status initialize() override{
        z_status status=Service::initialize();
        if (status)
            return status;

        gGpio.initialize();
        _leds={0,  &gGpio.ledRed,&gGpio.ledGreen,&gGpio.ledYellow      };

        gGpio.ledRed.flash(2);
        gGpio.ledGreen.flash(2);
        gGpio.ledYellow.flash(2);
        gBeepPwm.init();


        //TODO   -
        gBeepPwm._max_duty=_beep_max_volume;
        gBeepPwm._output_enable=_beep_enable;


        bat.init();
        gBeepPwm.pushNotes({
            {2000,10,2},
            {0,100,2},
            {2000,10,2},
        });

        reg_bin_func("setLed",&IoService::handleSetLed);
        reg_bin_func("ledFlash",&IoService::handleFlashLed);
        reg_func("stat",&IoService::get_status_json);
        reg_func("",&IoService::get_status_json);
        reg_func("beep",&IoService::get_beep_json);
        reg_func("takeOnMe",&IoService::takeOnMe);
        reg_func("config",&IoService::post_config);
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
    ZPROP(_battery_poll_interval);
    ZPROP(_max_beep_queue);

    ZPROP(_beep_enable);
    ZPROP(_beep_max_volume);
    ZOBJ(bat);
    ZOBJ_EX(gGpio,"gpio",ZFF_PROP_DEF,"global gpio object");
    ZOBJ_EX(gBeepPwm,"beeper",ZFF_PROP_DEF,"global beep PWM");
};

ROOT_SERVICE(IoService);
