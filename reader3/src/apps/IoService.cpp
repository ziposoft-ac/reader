//
// Created by ac on 7/17/26.
//
#include "pch.h"
#include "main/Service.h"
#include "battery//Battery.h"
#include "io/gpio.h"

#include "global.h"


struct Counter {
    U64 count;

};




class Box : public  Service{
public:
    Battery bat;
    //Gpio gpio;
    Box(){}
    virtual ~Box() {}
    z_status initialize() override{
        ZDBGS.add_stdout();
        bat.init();
        gGpio.initialize();
        return zs_ok;
    };
    z_status shutdown() override{
        gGpio.shutdown();

        return zs_ok;
    };
    U64 _counter=0;

};
ZMETA(Box) {
    ZBASE(Service);
    ZOBJ(bat);
    ZOBJ_EX(gGpio,"gpio",ZFF_PROP_DEF,"global gpio object");

};

ROOT_SERVICE(Box);
