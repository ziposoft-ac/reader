#include "IoApi.h"

#include "api/MqClient.h"

z_status ioLedSet(LedSet_t set) {
    return mq_send_msg_t<LedSet_t>(ioServiceName,"setLed",set );
}

z_status ioLedFlash(LedFlash_t flash) {
    return mq_send_msg_t<LedFlash_t>(ioServiceName,"ledFlash",flash );
}
