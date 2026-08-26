#include "LedApi.h"
#include "api/MqClient.h"






int foo()
{


    z_status s=apiLedService.LedFlash({LedGreen,1000,3});

    return s;
}

#define API LED_API

#include "api/ApiDefine.inc"
