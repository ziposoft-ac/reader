//
// Created by ac on 7/17/26.
//

#include "LedService.h"
#include "global.h"
ZMETA(LedServiceMq) {
    ZBASE(MqServer);
    ZACT(sendFlashLed);

};



ZMETA(LedService) {
    ZBASE(Service);
    ZOBJ(mq);
    ZACT(init);
};



LedService ledService;
MqApiHandler handler(&ledService);


int LedServiceMq::process_message(MqMsg* msg) {
    printf("LedServiceMq RX: %s\n",msg->command_str);

    handler.exec_callback(msg->command_str,(void*)msg->data);
    return 0;
}

z_status LedService::init() {

    handler.add_callback("flashLed",&LedService::flashLed);
        return zs_ok;

}


SET_ROOT_OBJ(ledService);