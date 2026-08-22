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





class IoClient : public  Service,public IoApiClient{
public:
    //BeepPwm beeper;
    IoClient(){}
    virtual ~IoClient() {}


};
ZMETA(IoClient) {
    ZBASE(IoApiClient);

    ZBASE(Service);

};

ROOT_SERVICE(IoClient);
