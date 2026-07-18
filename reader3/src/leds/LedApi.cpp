#include "LedApi.h"



#define API(CLASS,OBJ,LIST) struct api##CLASS_t { LIST } api##CLASS;
#define CMD(_X_) z_status call##_X_(const _X_& data);


LED_API

#undef CMD
#define CMD(X) { sizeof(X), #X,  }



int foo()
{


    apiLedService.callLedFlash({LedGreen,1000,3});
}