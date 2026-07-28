//
// Created by ac on 7/17/26.
//
#include "pch.h"
#include "main/Service.h"
#include "battery/Battery.h"
#include "io/gpio.h"
#include "web/WebServer.h"
#include "io/BeepPwm.h"
#include "io/IoApi.h"

#include "global.h"


struct Counter {
    U64 count;

};




class IoService : public  Service,public CommandHandler{
public:
    BeepPwm beeper;

    WebServer ws;
    MqServer mq;
    Battery bat;
    //Gpio gpio;
    IoService(){}
    virtual ~IoService() {}
    int handleSetLed(LedSet_t* set) {
        printf("setled:%d,%d\n",set->color,set->on);
        return zs_ok;
    }
    int handleFlashLed(LedFlash_t* set) {

        return zs_ok;
    }
    z_status setLed(int color, int onoff) {

        mq_send_msg_t<LedSet_t>(ioServiceName,"setLed",{
            (LedColor)color,(bool)onoff
        });
        return zs_ok;
    }
    z_status ledFlash(int color, int onoff) {

        mq_send_msg_t<LedSet_t>(ioServiceName,"setLed",{
            (LedColor)color,(bool)onoff
        });
        return zs_ok;
    }
    int get_status_json(z_string_map& params,z_json_stream& jout) {
        bat.json_get(jout);
        return 0;
    }
    z_status initialize() override{
        //ZDBGS.add_stdout();
        ws._port=8001;
        ws.start();
        bat.init();
        mq.run(ioServiceName);
        gGpio.initialize();
        reg_func("setLed",&IoService::handleSetLed);
        reg_func("stat",&IoService::get_status_json);
        reg_func("",&IoService::get_status_json);
        ws.register_consumer(this);
        mq.register_consumer(this);

        bat.start();
        return zs_ok;
    };
    z_status shutdown() override{
        ws.stop();
        mq.shutdown();
        gGpio.shutdown();
        bat.shutdown();

        return zs_ok;
    };
    U64 _counter=0;

};
ZMETA(IoService) {
    ZBASE(Service);
    ZOBJ(mq);
    ZOBJ(ws);
    ZOBJ(bat);
    ZOBJ_EX(gGpio,"gpio",ZFF_PROP_DEF,"global gpio object");
    ZCMD(setLed, ZFF_CMD_DEF, "setLed",
        ZPRM(int, color, 1, "color", ZFF_PARAM),
        ZPRM(int, on, 1, "onoff", ZFF_PARAM)
     );
    ZCMD(ledFlash, ZFF_CMD_DEF, "ledFlash",
        ZPRM(int, color, 1, "color", ZFF_PARAM),
        ZPRM(int, on, 1, "onoff", ZFF_PARAM)
 );
};

ROOT_SERVICE(IoService);
