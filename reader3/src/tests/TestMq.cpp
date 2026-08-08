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
    //MqServerCb<Tester> mq2;
    z_status rxMsg(MqMsg* msg) {

        printf("got msg %d,%s\n",msg->msg_id,msg->mq_reply_name);
        z_status status=mq_send_msg(msg->mq_reply_name,mq_command_pong,"",mq_data_none);
        Z_ERROR(status);
        return zs_ok;

    }
    z_status initialize() override{

        mq1.run("/test",this,&Tester::rxMsg);
        return zs_ok;
    };
    z_status shutdown() override{
        mq1.shutdown();
        return zs_ok;
    };
    U64 _counter=0;

    z_status runCounter(int max) {
        U64 count=0;

        while (count<=max) {



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
};
ZMETA(Tester) {
    ZBASE(Service);
    //ZOBJ(mq1);
    ZCMD(runCounter, ZFF_CMD_DEF, "runCounter",
         ZPRM(int, count, 10, "count", ZFF_PARAM)
         );
};

ROOT_SERVICE(Tester);

