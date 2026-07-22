//
// Created by ac on 7/17/26.
//

#ifndef ZIPOSOFT_SERVICE_H
#define ZIPOSOFT_SERVICE_H
#include "pch.h"
#include "global.h"

#include "api/MqServer.h"
class Service;






class Service {
public:
    Service() {
    }
    virtual ~Service() {}

    friend z_factory_t<Service>;
    virtual z_status shutdown();
    virtual z_status initialize();
    z_status run();

};


template <class SERVICE> Service* getRootServiceT(z_factory** pfactory) {
    static SERVICE service;
    *pfactory = GET_FACT(SERVICE);
    return &service;


}

Service* getRootService(z_factory** factory);

#define ROOT_SERVICE(_TYPE_) Service* getRootService(z_factory** factory) { return getRootServiceT<_TYPE_>(factory); }



ZMETA_DECL(Service) {
    ZACT(initialize);
    ZACT(shutdown);
    ZOBJ_EX(gConsole,"console",ZFF_PROP_DEF,"Console");
}

#endif //ZIPOSOFT_SERVICE_H
