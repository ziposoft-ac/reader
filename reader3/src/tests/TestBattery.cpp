//
// Created by ac on 7/17/26.
//
#include "pch.h"
#include "main/Service.h"
#include "battery//Battery.h"

#include "global.h"


struct Counter {
    U64 count;

};




class TestBattery : public  Service{
public:
    Battery bat;
    TestBattery(){}
    virtual ~TestBattery() {}
    z_status initialize() override{
        ZDBGS.add_stdout();
        return zs_ok;
    };
    z_status shutdown() override{
        return zs_ok;
    };
    U64 _counter=0;

};
ZMETA(TestBattery) {
    ZBASE(Service);
    ZOBJ(bat);

};

ROOT_SERVICE(TestBattery);
