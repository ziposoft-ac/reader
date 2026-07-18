//
// Created by ac on 7/17/26.
//

#include "BeepService.h"
#include "global.h"
ZMETA(BeepServiceMq) {
    ZBASE(MqServer);
};



ZMETA(BeepService) {
    ZBASE(Service);
    ZOBJ(mq);
};


BeepService beepService;

SET_ROOT_OBJ(beepService);