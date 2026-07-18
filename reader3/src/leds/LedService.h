//
// Created by ac on 7/17/26.
//

#ifndef ZIPOSOFT_SERVICELEDS_H
#define ZIPOSOFT_SERVICELEDS_H

#include "pch.h"

#include "util/Service.h"
#include "LedApi.h"





class LedServiceMq : public MqServer{
    friend z_factory_t<LedServiceMq>;

private:

public:

    LedServiceMq() {
        _this_mq_server_name="/LedService";
    }
    virtual  int process_message(MqMsg* msg) ;


    z_status sendFlashLed() {
         LedFlash flash={
             LedGreen,1000,4
         };
        send_msg(_this_mq_server_name,"flashLed",(ctext)&flash,sizeof(flash));
        return zs_ok;

    }
};


class LedService : public  Service{
    public:

    LedService(){}
    virtual ~LedService() {}

    LedServiceMq mq;
    z_status init();
    z_status flashLed(LedFlash* cmd) {
        printf("flashLed !! %d,%d\n",
            cmd->color,cmd->count
            );
        return zs_ok;
    }
};


#endif //ZIPOSOFT_SERVICELEDS_H
