//
// Created by ac on 7/17/26.
//
#include "pch.h"
#include "main/Service.h"

#include "global.h"
#include "api/CommandHandler.h"


struct Counter {
    U64 count;

};




class Tester : public  Service,public  CommandHandler{
public:

    Tester(){}
    virtual ~Tester() {}
    MqServer mq1;
    MqServer mq2;
    z_status initialize() override{
        reg_bin_func("count",&Tester::handlerCount);
        mq1.register_consumer(this);
        mq2.register_consumer(this);
        return zs_ok;
    };
    z_status shutdown() override{
        mq1.shutdown();
        mq2.shutdown();
        return zs_ok;
    };
    U64 _counter=0;

    z_status runCounter(z_string dest,int max) {
        U64 count=0;
        U64 us_start= z_time_get_ticks_us();



        while (count<=max) {
            z_status s=mq_send_msg_t<Counter>(dest,"count",{count});
            if (s==zs_device_busy) {
                z_sleep_us(100);
                continue;
            }
            if (s!=zs_ok) {
                Z_ERROR_LOG("sending count failed\n");
                perror("mq error");
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
    z_status handlerCount(Counter* data) {
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
        z_status s=zs_not_implemented;

        return s;
    }
    z_status Dummy() {
        z_status s=zs_not_implemented;

        return s;
    }
};
ZMETA(Tester) {
    ZBASE(Service);
    ZOBJ(mq1);
    ZOBJ(mq2);
    ZCMD(runCounter, ZFF_CMD_DEF, "runCounter",
         ZPRM(z_string, dest, "/mq1", "count", ZFF_PARAM),
         ZPRM(int, count, 10, "count", ZFF_PARAM)
         );
};

ROOT_SERVICE(Tester);

