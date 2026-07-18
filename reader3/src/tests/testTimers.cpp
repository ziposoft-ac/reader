//
// Created by ac on 11/13/20.
//

#include "tests.h"
#include "../root.h"
#include "zipolib/z_error.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/eventfd.h>
ZMETA(TestTimerStress)
{
    ZBASE(Test);
    ZPROP(_delay_max);
    ZPROP(_delay_min);
    ZPROP(_num_timers);
    ZPROP(_step);
    ZPROP(_spawn);
    ZPROP(_max_total_callbacks);

};
ZMETA(TestTimerInterval)
{
    ZBASE(TestTimer);
};
ZMETA(TestTimerSimple)
{
    ZBASE(TestTimer);
};
ZMETA(TestTimerCascade)
{
    ZBASE(TestTimer);
};

z_status TestTimer::stop()
{
    if(_timer)
        _timer->stop();
    onStop();
    _running=false;

    return zs_ok;
}
z_status TestTimer::start()
{
    _current_iteration=0;
    z_status status=onStart();
    if(status!=zs_ok) {
        Z_ERROR_MSG(status,"start timer failed");
        return status;

    }



    if(!_timer)
        _timer=gTimerService.create_timer_t(this,&TestTimer::timer_callback,0    );
    _timer->start(_interval);
    return zs_ok;
}



int TestTimer::timer_callback(void* context)
{
    int ms=onCallback(context);
    if(!ms)
    {
        _running=false;
        return 0;
    }
    return ms;
}


int TestTimerCascade::onCallback(void *p) {

    printf("count=%d\n",_current_iteration--);
    if (_current_iteration==4) {
        _timer2->start(3000,true);
    }
    if (_current_iteration>0)
        return _interval;


    return 0;

}

int TestTimerCascade::timer_callback2(void *) {
    printf("timer_callback2\n");

    return 0;
}

z_status TestTimerCascade::onStop() {
    if(_timer2)
        _timer2->stop();

    return zs_ok;
}

z_status TestTimerCascade::onStart() {
    _current_iteration=_iterations;
    if(!_timer2)
        _timer2=gTimerService.create_timer_t(this,&TestTimerCascade::timer_callback2,0    );
    _timer2->_debug=true;
    return zs_ok;


}


int  TestTimerInterval::onCallback(void*)
{
    // zout <<  "TestWsBlast::onCallback\n";
    _iterations--;
    _current_iteration++;
    if(_current_iteration*_interval>_print_interval_seconds*1000)
    {
        _current_iteration=0;
        zout << "timertest:"<< _iterations<< " every 5 seconds\n";

    }
    _last_iter=_iterations;

    return _interval;
}

int TestTimerStress::callback(void* vctx) {

    TestTimerContext* ctx=(TestTimerContext*)vctx;
    ctx->_counter++;


    if (_callback_count++>_max_total_callbacks) {
        printf("max callbacks reached, stopping\n");
        return 0;

    }
    printf("timer# %d callback,count=%d,total=%d\n",ctx->_id,ctx->_counter,_callback_count);

    int i;
    for (i=0;i<_spawn;i++) {
        int id=(ctx->_id + i + _step)%_num_timers;
        TestTimerContext* other;
        if (!_timers.get(id,other)) {
            Z_ERROR_MSG(zs_internal_error,"timer get failed");
        }
        other->_timer->start(50+id,true);



    }

    return zs_ok;
}


z_status TestTimerStress::stop() {

    TestTimerContext* ctx;
    while (ctx=_timers.pop_first()) {
        ctx->_timer->stop();
        delete ctx;

    }
    return zs_ok;
}

z_status TestTimerStress::start() {
    int i=0;
    if (!_num_timers)
        return zs_bad_parameter;
    if (_running)
        return zs_already_open;
    stop();
    for (i=0;i<_num_timers;i++) {
        auto ctx=new TestTimerContext(i);
        ctx->_timer=gTimerService.create_timer_t(this,&TestTimerStress::callback,ctx    );

        _timers.add(i,ctx);

    }
    _callback_count=0;

    _timers.getobj(0)->_timer->start(10);
    return zs_ok;


}