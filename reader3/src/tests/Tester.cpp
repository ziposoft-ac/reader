//
// Created by ac on 7/17/26.
//
#include "pch.h"
#include "main/Service.h"
#include "tests/tests.h"
#include "global.h"
#include "api/CommandHandler.h"
#include "rfid/cfmu804.h"
#include "io/gpio.h"

struct Counter {
    U64 count;

};




class Tester : public  Service,public  CommandHandler{
public:
    Tests tests;
    Cfmu804 cfmu804;

    Tester(){}
    virtual ~Tester() {}
    z_status initialize() override{
        gGpio.initialize();

        return zs_ok;
    };
    z_status shutdown() override{
        return zs_ok;
    };
};
ZMETA(Tester) {
    ZBASE(Service);
    ZOBJ(tests);
};

ROOT_SERVICE(Tester);

RfidReader& getRfidReader() {
    return gTester.cfmu804;
}
void gCallbackVisitNotify(){}

