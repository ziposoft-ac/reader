//
// Created by ac on 11/2/20.
//#include "pch.h"
#include "pch.h"
#include "timers.h"
#include <climits>


// Need for more than one timer service?
TimerService gTimerService;


ZMETA(TimerService)
{
    ZACT(stop);
    ZACT(test);

};


Timer::Timer(TimerService *service, TimerCallback callback, void* user_context)
{
    _service=service;
    _user_callback=callback;
    _user_context=user_context;
}

Timer::~Timer()
{




}
U64 Timer::update()
{
    if (!_running)
        return 0;

    if(!_ts_expire) {
        return 0; //not running

    }
    U64 ts_now=z_time::get_now_ms();
    if (_debug) {
        int dummy=2;
        dummy=1;
    }
    U64 ms_left=0;
    if(_ts_expire> ts_now)
        return _ts_expire;
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
void Timer::start() {
    //TODO calling start if already running?
    if (_running)
        return;
    _service->timer_start(this,_interval,false);
}
void Timer::start(int ms,bool reset) {

    _service->timer_start(this,ms,reset);

}
int Timer::invoke_callback()
{
    return (*_user_callback)(_user_context);
}

TimerService::~TimerService()
{
    stop();
    for (auto i : _timers)
    {
        delete i;
    }
}

TimerService::TimerService()
{

}
z_status TimerService::stop()
{
    std::unique_lock<std::mutex> mlock(_mutex_sync);

    if(_running)
    {
        _running=false;

        _cond_stop_wait.notify_all();
    }
    if (_thread_process.joinable())
        _thread_process.join();
    return zs_ok;
}
bool TimerService::remove_timer(Timer *timer) {
    std::unique_lock<std::mutex> mlock(_mutex_sync);

    timer->stop();
    _timers.erase(timer);
    delete timer;
    return true;
}
Timer* TimerService::createTimer(TimerCallback callback, void* user_data, int ms_expire )
{


    Timer* timer=z_new Timer(this,callback,user_data);

    {
        std::unique_lock<std::mutex> mlock(_mutex_sync);
        _timers.insert(timer);
    }
    timer_start(timer,ms_expire,true);
    return timer;
}


z_time test_time;
int test_callback(void* data)
{
    U64 ms=test_time.get_elapsed_ms()%100000;

    size_t interval;
    interval = (size_t) data;
    zout.format_append("callback %d: %05d\n",interval,ms);

    return interval*1000;

}
z_status TimerService::test()
{
    test_time.set_now();
    createTimer(test_callback,(void*)1,100);
    createTimer(test_callback,(void*)2,200);
    createTimer(test_callback,(void*)3,300);

    return zs_ok;
}

/*
 * This is called by Timer::start to start the thread if necessary.
 * The timer service cannot run if there are no timers
 */
z_status TimerService::timer_start(Timer* t,int ms,bool reset)
{

    bool external_context=false;
    std::unique_lock<std::mutex> lock(_mutex_sync, std::defer_lock);
    if (std::this_thread::get_id()!=_thread_id) {
        lock.lock();
        external_context=true;
    }
    else {
        _flag_reprocess_timers=true;
    }
    //std::unique_lock<std::mutex> mlock(_mutex_sync);
    t->_running=true;
    if(reset || (t->_ts_expire==0))
    {
        t->_interval=ms;
        t->_ts_expire=ms+z_time::get_now_ms();
    }
    // If called from the timer callback loop, then exit
    // The _flag_reprocess_timers is set to apply new timer

    if (!external_context)
        return zs_ok;

    if(_running) {
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
    _running=true;
    _thread_process = std::thread(&TimerService::process_thread, this);
    _thread_id=_thread_process.get_id();

    return zs_ok;
}

void Timer::restart(int ms) {
}

void TimerService::timer_stop(Timer *t) {

    std::unique_lock<std::mutex> lock(_mutex_sync, std::defer_lock);
    if (std::this_thread::get_id()!=_thread_id) {
        lock.lock();
    }
    else {
        _flag_reprocess_timers=true;

    }
    //std::unique_lock<std::mutex> mlock(_mutex_sync);
    t->_running=false;
    t->_ts_expire=0;
}



bool TimerService::update_timers()
{
    std::unique_lock<std::mutex> mlock(_mutex_sync);

    while (1) {
        _ts_next_expire=0;;

        _flag_reprocess_timers=false;
        for (auto i : _timers)
        {
            U64 t=i->update();
            if (t) {
                if ((_ts_next_expire==0)||(t<_ts_next_expire)) {
                    _ts_next_expire=t;

                }

            }

        }
        if (!_flag_reprocess_timers)
            break;

    }


    if (!_ts_next_expire) {
        _running=false;
        return false;

    }

    return true;


}
void TimerService::process_thread()
{
    try {
        int ms_elapsed=0;
        while(_running)
        {
            //ZDBG("process_thread\n");

            if (!update_timers())
                return;
            U64 ms_wait=_ts_next_expire-z_time::get_now_ms();
            if (ms_wait) {
                std::unique_lock<std::mutex> m_wait(_mutex_stop_wait);
                //ZDBG("waiting for %d ms\n",ms_next_wait);
                if(_cond_stop_wait.wait_for(m_wait,std::chrono::milliseconds (ms_wait)) !=std::cv_status::timeout) {

                }
            }
        }
    }
    catch (std::exception &e)
    {
        Z_ERROR_MSG(zs_internal_error,"Timer Service Thread exception: %s",e.what());
        std::cout << "Type:    " << typeid(e).name() << "\n";
        _running=false;
    }


}