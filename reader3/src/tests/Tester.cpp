//
// Created by ac on 7/17/26.
//
#include "pch.h"
#include "main/Service.h"
#include "leds/LedApi.h"

#include "global.h"


struct Counter {
    U64 count;

};

#define TEST_API \
API_NAME(Tester, \
CMD(Counter)    \
)

#define API TEST_API
#include "api/ApiDeclare.inc"
#define API TEST_API

#include "api/ApiDefine.inc"




class Tester : public  Service{
public:

    Tester(){}
    virtual ~Tester() {}
    MqServerMap<Tester> mq1;
    MqServerMap<Tester> mq2;
    z_status initialize() override{

        mq1._object=this;
        mq2._object=this;
        //mq1.run_map("/test1",this);
        //mq2.run_map("/test2",this);
        return zs_ok;
    };
    z_status shutdown() override{
        mq1.shutdown();
        mq2.shutdown();
        return zs_ok;
    };
    U64 _counter=0;

    z_status runCounter(int max) {
        U64 count=0;
        U64 us_start= z_time_get_ticks_us();



        while (count<=max) {

            if (apiTester.Counter({count})!=zs_ok) {
                Z_ERROR_LOG("sending count failed\n");
                break;
            }

            count++;
            if (count%100000 ==0) {
                printf("count=%d\n",count);
            }
        }
        U64 us_end= z_time_get_ticks_us();
        U64 diff=us_end-us_start;
        double calls_per_sec=max/((double)diff/1000000.0);
        printf("%d iters in %lu us, %lu per call, %lf calls per sec \n",max,diff,diff/max,calls_per_sec);
        return zs_ok;
    }
    z_status Counter(Counter* data) {
        U64 countIn=data->count;

        if (countIn) {
            if (countIn!=_counter) {
                Z_ERROR_LOG("Count does not match: %lld != %lld\n", countIn, _counter);
            }
            if (_counter%100000 ==0) {
                printf("_counter=%d\n",_counter);
            }
        }
        else {
            _counter=0;
            printf("starting count at 0\n");
        }
        _counter++;
        return zs_ok;
    }
    z_status LedFlash() {
        z_status s=apiLedService.LedFlash({LedGreen,1000,3});

        return s;
    }
    z_status Dummy() {
        z_status s=apiLedService.Dummy({1000});

        return s;
    }
};
ZMETA(Tester) {
    ZBASE(Service);
    ZOBJ(mq1);
    ZOBJ(mq2);
    ZCMD(runCounter, ZFF_CMD_DEF, "runCounter",
         ZPRM(int, count, 10, "count", ZFF_PARAM)
         );
};
ZMETA(MqServerMap<Tester>) {
    ZBASE(MqServer);
};

ROOT_SERVICE(Tester);

#define MQ_HANDLER Tester
#define API TEST_API
#include "api/ApiMap.inc"