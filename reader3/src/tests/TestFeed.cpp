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
    Timer* _timer_counter=0;
    Timer* _timer_stat=0;
    U64 _counter=0;
    int _timer_interval=1000;
    int _stat_interval=1000;
    bool _running=false;
    z_status initialize() override{

        return start_feed();;
    };
    z_status start_feed() {
        if (_running)
            return zs_ok;
        mq.run("/feedtest");
        mq.register_consumer(this);

        if (!_timer_stat)
            _timer_stat=CREATE_TIMER(TestSubscribe::timer_stats,0 ,_stat_interval   );
        _running=true;

        return zs_ok;
    };
    z_status start_counter() {
        if (!_timer_counter)
            _timer_counter=CREATE_TIMER(TestSubscribe::counter_callback   );
        _timer_counter->start(_timer_interval);
        return zs_ok;
    };
    z_status stop_counter() {

        _timer_counter->stop();
        ZDBG("stopped\n");
        return zs_ok;
    };
    int timer_stats(void* ptr) {
        static int last=0;
        ZDBG("num subscribers %d\n",mq.num_subscribers());

        ZDBG("counter %d\n",_counter);
        double diff=_counter-last;
        ZDBG("per sec %0.1lf\n",diff/_stat_interval *1000);
        last=_counter;

        return _stat_interval;
    }
    int counter_callback(void* ptr) {

        z_json_str_stream js;
        js.obj_start();
        js.keyval_int("count",_counter);

        js.obj_end();
        _counter++;
        mq.publish("count",js.as_string());
        return _timer_interval;
    }
    z_status shutdown() override {

        gTimerService.remove_timer(_timer_counter);
        _timer_counter=0;
        return stop_feed();
    }
    z_status stop_feed() {
        if (!_running)
            return zs_ok;
        ZDBG("mq.shutdown\n");
        gTimerService.remove_timer(_timer_stat);
        ZDBG("stopped\n");
        mq.shutdown();
        ZDBG("mq.remove_consumer\n");

        mq.remove_consumer(this);
        ZDBG("mq.remove_timer\n");



        _running=false;
        return zs_ok;
    };

    z_status send_count(int max) {
        U64 count=0;

        return zs_ok;
    }
    z_status single() {
        counter_callback(0);
        return zs_ok;
    }
};
ZMETA(TestSubscribe) {
    ZBASE(Service);
    ZOBJ(mq);
    ZACT(start_feed);
    ZACT(stop_feed);
    ZACT(single);
    ZCMD(send_count, ZFF_CMD_DEF, "setLed",
     ZPRM(int, color, 1, "color", ZFF_PARAM)
    );
    ZACT(start_counter);
    ZACT(stop_counter);
    ZPROP(_timer_interval);
    ZPROP(_stat_interval);
};

ROOT_SERVICE(TestSubscribe);