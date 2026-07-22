//
// Created by ac on 7/17/26.
//

#ifndef ZIPOSOFT_SERVICELEDS_H
#define ZIPOSOFT_SERVICELEDS_H

#include "pch.h"

#include "../main/Service.h"
#include "LedApi.h"






class LedService : public  Service{
    public:

    LedService(){}
    virtual ~LedService() {}
    MqServerMap<LedService> mq;
    z_status initialize() override;
    z_status shutdown() override;


    z_status Dummy(Dummy* cmd) {
        printf("Dummy !! %d\n",
            cmd->dummy
            );
        return zs_ok;
    }
    z_status LedFlash(LedFlash* cmd) {
        printf("flashLed !! %d,%d\n",
            cmd->color,cmd->count
            );
        return zs_ok;
    }
    z_status LedSet(LedSet* cmd) {
        printf("LedSet !! %d,%d\n",
            cmd->color,cmd->on
            );
        return zs_ok;
    }
    z_status test() {
        z_status s=apiLedService.LedFlash({LedGreen,1000,3});

        return s;
    }
};


#endif //ZIPOSOFT_SERVICELEDS_H
