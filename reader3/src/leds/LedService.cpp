//
// Created by ac on 7/17/26.
//

#include "LedService.h"
#include "../global.h"
ZMETA(LedService) {
    ZBASE(Service);
    ZOBJ(mq);

    ZACT(test);
};


ZMETA(MqServerMap<LedService>) {
    ZBASE(MqServer);
};

#define MQ_HANDLER LedService
#define API LED_API
#include "api/ApiMap.inc"




z_status LedService::initialize() {

    mq.run_map("/LedService",this);

    return zs_ok;

}
z_status LedService::shutdown() {

    mq.stop();
    return zs_ok;

}


ROOT_SERVICE(LedService);