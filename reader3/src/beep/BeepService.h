//
// Created by ac on 7/17/26.
//

#ifndef ZIPOSOFT_SERVICELEDS_H
#define ZIPOSOFT_SERVICELEDS_H

#include "pch.h"

#include "main/Service.h"
#include "io/BeepPwm.h"



class BeepService : public  Service{
    public:

    virtual ~BeepService() {}
    BeepPwm beeper;

    MqServerMap<BeepService> mq;
};


#endif //ZIPOSOFT_SERVICELEDS_H
