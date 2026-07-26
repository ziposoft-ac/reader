//
// Created by ac on 7/17/26.
//
#include "pch.h"
#include "main/Service.h"

#include "global.h"
#include "api/CommandHandler.h"

#include "util/timers.h"

class Subscriber {
public:
    z_string _name;
    Subscriber(ctext name) : _name(name) {

    }

};



class TestSubscribe : public  Service,public CommandHandler{
public:

    TestSubscribe(){}
    virtual ~TestSubscribe() {}
    MqFeed mq;
    Timer* _timer=0;
    U64 _counter=0;
    int _timer_interval=1000;
    bool _running=false;
    z_status initialize() override{

        return start();;
    };
    z_status start() {
        if (_running)
            return zs_ok;
        mq.run("/feedtest");
        mq.register_consumer(this);
        if (!_timer)
        _timer=gTimerService.create_timer_t(this,&TestSubscribe::timer_callback,0 ,_timer_interval   );
        _running=true;

        return zs_ok;
    };
    int timer_callback(void* ptr) {

        z_json_str_stream js;
        js.obj_start();
        js.keyval_int("count",_counter);

        js.obj_end();
        _counter++;
        ZDBG("timer callback %d\n",_counter);
        mq.publish("count",js.as_string());
        return _timer_interval;
    }
    z_status shutdown() override {
        return stop();
    }
    z_status stop() {
        if (!_running)
            return zs_ok;
        ZDBG("mq.shutdown\n");

        mq.shutdown();
        ZDBG("mq.remove_consumer\n");

        mq.remove_consumer(this);
        ZDBG("mq.remove_timer\n");

        gTimerService.remove_timer(_timer);
        ZDBG("stopped\n");
        _timer=0;

        _running=false;
        return zs_ok;
    };

    z_status runCounter(int max) {
        U64 count=0;

        return zs_ok;
    }
    z_status LedFlash() {

        return zs_ok;
    }
};
ZMETA(TestSubscribe) {
    ZBASE(Service);
    ZOBJ(mq);
    ZACT(start);
    ZACT(stop);
};

ROOT_SERVICE(TestSubscribe);