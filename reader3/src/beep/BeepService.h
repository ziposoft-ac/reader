//
// Created by ac on 7/17/26.
//

#ifndef ZIPOSOFT_SERVICELEDS_H
#define ZIPOSOFT_SERVICELEDS_H

#include "pch.h"

#include "../util/Service.h"

class BeepServiceMq : public MqServer{
    friend z_factory_t<BeepServiceMq>;

private:

public:
    BeepServiceMq() {
        _this_mq_server_name="/BeepService";
    }
    virtual  int process_message(MqMsg* msg) {
        printf("BeepServiceMq RX: %s\n",msg->command_str);
        return 0;
    }


};


class BeepService : public  Service{
    public:

    virtual ~BeepService() {}

    BeepServiceMq mq;
};


#endif //ZIPOSOFT_SERVICELEDS_H
