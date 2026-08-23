//
// Created by ac on 11/2/20.
//#include "pch.h"
#include "pch.h"
#include "timers.h"
#include <climits>


// Need for more than one timer service?
TimerService gTimerService;

#ifdef DEBUGL
#undef DBGL
U64 DBG_TS_START=z_time::get_now_ms();

static double get_dbg_ts_ms(U64 ts) {
    double diff = ts - DBG_TS_START;
    return diff / 1000;
}

#define DBGL(...) { U64 now=z_time_get_ticks_ms();get_debug_logger().time_mark(now-DBG_TS_START);get_debug_logger().format_append(__VA_ARGS__); ZDBGS<<'\n';   }

#else
#define DBGL(...)

#endif



ZMETA(TimerService)
{
    ZACT(shutdown);

};


Timer::Timer(ctext name,TimerService *service, TimerCallback callback, void* user_context)
{
    _name=name;
    _service=service;
    _user_callback=callback;
    _user_context=user_context;
}

Timer::~Timer()
{




}
U64 Timer::_update()
{
    if (!_running) {

        DBGL("%s not running",_name.c_str());
        return 0;

    }

    if(!_ts_expire) {
        DBGL("%s no  expire?",_name.c_str());

        return 0; //not running

    }
    U64 ts_now=z_time::get_now_ms();

    U64 ms_left=0;
    if(_ts_expire> ts_now)
        return _ts_expire;

    DBGL("%s  invoke_callback",_name.c_str());

    ms_left=invoke_callback();
    if (z_time::get_now_ms()-ts_now > 200) {
        ZDBG("Timer callback took more than 200 milliseconds\n");
    }

    if (!ms_left){
        _running=false;
        _ts_expire=0;
    } else {
        _ts_expire=ts_now+ms_left;
    }
    return _ts_expire;
}

// TODO - possible race condition !!
// TimerSerice could be updating timer while user calls start/stop
void Timer::stop() {
    _service->timer_stop(this);

    _running=false;
    _ts_expire=0;
}

void Timer::start_ms_if_not_running(U32 ms) {
    _service->_timer_start_ms(this,ms,timer_start_if_not_running);

}

void Timer::start_ms_if_sooner(U32 ms) {
    _service->_timer_start_ms(this,ms,timer_start_if_earlier);

}

void Timer::start_ms_reset(U32 ms) {
    _service->_timer_start_ms(this,ms,timer_start_reset);

}

void Timer::start_ts_if_not_running(const z_time& ts) {
    _service->_timer_start_ts(this,ts,timer_start_if_not_running);

}

void Timer::start_ts_if_sooner(const z_time& ts) {
    _service->_timer_start_ts(this,ts,timer_start_if_earlier);

}

void Timer::start_ts_reset(const z_time& ts) {
    _service->_timer_start_ts(this,ts,timer_start_reset);

}

int Timer::invoke_callback()
{
    return (*_user_callback)(_user_context);
}

TimerService::~TimerService()
{
    shutdown();
    for (auto i : _timers)
    {
        delete i;
    }
}

TimerService::TimerService()
{

}
z_status TimerService::shutdown()
{
    if (!_timer_service_running)
        return zs_ok;
    std::unique_lock mlock(_mutex_sync);

    _timer_service_running=false;
    _cond_stop_wait.notify_all();
    if (_thread_process.joinable())
        _thread_process.join();
    return zs_ok;
}
bool TimerService::remove_timer(Timer *timer) {
    if (!timer)
        return false;
    // stop locks itself
    timer->stop();

    std::unique_lock mlock(_mutex_sync);

    _timers.erase(timer);
    delete timer;
    return true;
}
Timer* TimerService::createTimer(ctext name,TimerCallback callback, void* user_data, int ms_expire )
{


    Timer* timer=z_new Timer(name,this,callback,user_data);

    {
        std::unique_lock mlock(_mutex_sync);
        _timers.insert(timer);
    }
    _timer_start_ms(timer,ms_expire,timer_start_reset);
    return timer;
}


z_time test_time;
int test_callback(void* data)
{
    U64 ms=test_time.ms_since()%100000;

    size_t interval;
    interval = (size_t) data;
    zout.format_append("callback %d: %05d\n",interval,ms);

    return interval*1000;

}

z_status TimerService::_timer_start_ms(Timer *timer, U32 ms, timer_operation_t cond) {
    return _timer_start_ts(timer,ms+z_time::get_now_ms(),cond);
}
bool TimerService::_process_work(timer_start_work_t w) {


    if (w.oper==timer_oper_delete)
    {
        //TODO
        DBGL("DELETE- TODO %s",w.t->_name.c_str());

        return false;
    }
    if (w.oper==timer_oper_stop)
    {
        DBGL("STOP %s",w.t->_name.c_str());

        if (w.t->is_running()) {
            w.t->_running=false;
            w.ts=0;
            return true;
        }
        return false;

    }
    if (w.t->_running ) {
        if (w.oper==timer_start_if_not_running) {
            DBGL("START, already running %s",w.t->_name.c_str());
            return false;

        }

        if (w.oper==timer_start_if_earlier) {
            if (w.ts.in_ms() >= w.t->_ts_expire) {
                DBGL("START, expiring sooner %s",w.t->_name.c_str());

                return false;
            }
        }
    }

    //std::unique_lock mlock(_mutex_sync);
    w.t->_running=true;
    w.t->_ts_expire=w.ts.in_ms();
    DBGL("START: %s, expiring at %6.3lf",w.t->_name.c_str(),get_dbg_ts_ms(w.t->_ts_expire));

    return true;

}
z_status TimerService::_timer_start_ts(Timer *t, const z_time &ts, timer_operation_t cond) {
    if (!_timer_service_running)
        init();
    _work_queue.push({t,ts,cond});
    _cond_stop_wait.notify_all();


    return zs_ok;
}
z_status TimerService::_timer_start_ts_orig(Timer *t, const z_time &ts, timer_operation_t cond) {


    bool external_context=false;
    std::unique_lock lock(_mutex_sync, std::defer_lock);
    if (std::this_thread::get_id()!=_thread_id) {
        lock.lock();
        external_context=true;
    }
    else {
        _flag_reprocess_timers=true;
    }
    if (t->_running && _timer_service_running) {
        if (cond==timer_start_if_not_running)
            return zs_no_change;

        if (cond==timer_start_if_earlier) {
            if (ts.in_ms() >= t->_ts_expire) {
                return zs_no_change;
            }
        }
    }
    //std::unique_lock mlock(_mutex_sync);
    t->_running=true;
    t->_ts_expire=ts.in_ms();

    // If called from the timer callback loop, then exit
    // The _flag_reprocess_timers is set to apply new timer

    if (!external_context)
        return zs_ok;

    if(_timer_service_running) {
        if (_ts_next_expire>t->_ts_expire) {
            // If new timer starts sooner than wait loop
            //force restart of loop
            _cond_stop_wait.notify_all();
        }
        return zs_ok;
    }
    // If the serive is not running then start it
    if (_thread_process.joinable())
        _thread_process.join();
    _timer_service_running=true;
    _thread_process = std::thread(&TimerService::process_thread, this);
    _thread_id=_thread_process.get_id();

    return zs_ok;
}
z_status TimerService::init() {
    if (_timer_service_running)
        return zs_already_open;
    // If the serive is not running then start it
    if (_thread_process.joinable())
        _thread_process.join();
    _timer_service_running=true;
    _thread_process = std::thread(&TimerService::process_thread, this);
    _thread_id=_thread_process.get_id();
    return zs_ok;


}

void Timer::restart(int ms) {
}

void TimerService::timer_stop(Timer *t) {

    std::unique_lock lock(_mutex_sync, std::defer_lock);
    if (std::this_thread::get_id()!=_thread_id) {
        lock.lock();
    }
    else {
        _flag_reprocess_timers=true;

    }
    //std::unique_lock mlock(_mutex_sync);
    t->_running=false;
    t->_ts_expire=0;
}



bool TimerService::update_timers()
{
    std::unique_lock mlock(_mutex_sync);

    bool timers_running=false;

    for (auto i : _timers)
    {
        i->_update();
    }
    timer_start_work_t work;
    while (_work_queue.pop(work)) {
        _process_work(work);

    }




    _ts_next_expire=0;;

    for (auto i : _timers)
    {
        if (i->_running) {
            U64 t=i->_ts_expire;
            if (t) {
                if ((_ts_next_expire==0)||(t<_ts_next_expire)) {
                    _ts_next_expire=t;
                    timers_running=true;

                }
            }

        }

    }



    return timers_running;


}


void TimerService::process_thread()
{
    try {

        while (_timer_service_running) {

            bool running=update_timers();


            U64 now=z_time_get_ticks_ms();


            if (running && (_ts_next_expire<=now))
                continue;

            if (running) {
                DBGL("Timers running,_ts_next_expire=%6.3lf ",get_dbg_ts_ms(_ts_next_expire));

                U64 ms_wait=_ts_next_expire-now;
                if (ms_wait) {
                    std::unique_lock m_wait(_mutex_stop_wait);
                    //ZDBG("waiting for %d ms\n",ms_next_wait);
                    if(_cond_stop_wait.wait_for(m_wait,std::chrono::milliseconds (ms_wait)) !=std::cv_status::timeout) {

                    }
                }

            }
            else {
                DBGL("No timers running waiting for new start");
                std::unique_lock m_wait(_mutex_stop_wait);
                //ZDBG("waiting for %d ms\n",ms_next_wait);
                _cond_stop_wait.wait(m_wait);


            }




        }

    }
    catch (std::exception &e)
    {
        Z_ERROR_MSG(zs_internal_error,"Timer Service Thread exception: %s",e.what());
        std::cout << "Type:    " << typeid(e).name() << "\n";
        _timer_service_running=false;
    }


}
