//
// Created by ac on 7/17/26.
//
#include "pch.h"
#include "main/Service.h"

#include "global.h"


struct Counter {
    U64 count;

};




class Tester : public  Service{
public:

    Tester(){}
    virtual ~Tester() {}
    MqServerCb<Tester> mq1;
    MqServerCb<Tester> mq2;
    z_status initialize() override{

        mq1.run("/t1",this);
        mq2.run("/t2");
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

        while (count<=max) {

            if (apiTester.Counter({count})!=zs_ok) {
                Z_ERROR_LOG("sending count failed\n");
                break;
            }

            count++;
            if (count%10000 ==0) {
                printf("count=%d\n",count);
            }
        }
        return zs_ok;
    }
    z_status Counter(Counter* data) {
        U64 countIn=data->count;

        if (countIn) {
            if (countIn!=_counter) {
                Z_ERROR_LOG("Count does not match: %lld != %lld\n", countIn, _counter);
            }
            if (_counter%10000 ==0) {
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