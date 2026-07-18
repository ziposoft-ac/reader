//
// Created by ac on 7/17/26.
//

#ifndef ZIPOSOFT_SERVICE_H
#define ZIPOSOFT_SERVICE_H
#include "pch.h"

#include "api/MqServer.h"
class Service;

extern Service *gService;

class Service {
public:
    Service() {
        gService=this;
    }
    virtual ~Service() {}

    friend z_factory_t<Service>;
    z_status shutdown();
    z_status initialize();
    z_status run();

};
ZMETA_DECL(Service) {

}

#endif //ZIPOSOFT_SERVICE_H
