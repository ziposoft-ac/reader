//
// Created by ac on 11/2/20.
//#include "pch.h"
#include "pch.h"
#include "timers.h"
#include <climits>


ZMETA(TimerService)
{
    ZACT(stop);
    ZACT(start);
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
int Timer::update(int ms_elapsed)
{
    if (!_running)
        return 0;

    if(!_ms_left) {
        return 0; //not running

    }
    if(_ms_left> ms_elapsed)
    {
        _ms_left-=ms_elapsed;
    } else{
        U64 start=z_time::get_now_ms();
        _ms_left=invoke_callback();
       if (z_time::get_now_ms()-start > 200) {
           ZDBG("Timer callback took more than 200 milliseconds\n");
       }

        if (!_ms_left)
            _running=false;
    }
    return _ms_left;
}

// TODO - possible race condition !!
// TimerSerice could be updating timer while user calls start/stop
void Timer::stop() {
    _running=false;
    _ms_left=0;
}
void Timer::start() {
    //TODO calling start if already running?
    if (_running)
        return;
    _running=true;

    _ms_left=_interval;
    _service->start();
}
void Timer::start(int ms,bool reset) {
    _running=true;

    if(reset || (_ms_left==0))
    {
        _ms_left=ms;
        _interval=ms;

    }
    _service->start();

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
        _cond_quit.notify_all();
        _running=false;
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
Timer* TimerService::createTimer(TimerCallback callback, void* user_data, int start )
{
    Timer* timer=z_new Timer(this,callback,user_data);

    {
        std::unique_lock<std::mutex> mlock(_mutex_sync);
        _timers.insert(timer);
    }

    if(start)
        timer->start(start);
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

z_status TimerService::start()
{
    if(_running) return zs_ok;
    std::unique_lock<std::mutex> mlock(_mutex_sync);
    if (_thread_process.joinable())
        _thread_process.join();
    _running=true;
    _thread_process = std::thread(&TimerService::process_thread, this);
    return zs_ok;
}
int TimerService::update_timers(int ms_elapsed)
{
    std::unique_lock<std::mutex> mlock(_mutex_sync);

    int ms_next_wait=INT_MAX;
    for (auto i : _timers)
    {
        int t=i->update(ms_elapsed);
        if(t && (t < ms_next_wait))
        {
            ms_next_wait=t;
        }
    }
    return ms_next_wait;


}
void TimerService::process_thread()
{
    try {
        int ms_elapsed=0;
        while(_running)
        {
            //ZDBG("process_thread\n");

            int ms_next_wait=update_timers(ms_elapsed);
            if(ms_next_wait==INT_MAX)
            {
                _running=false;
                return;
            }

            std::unique_lock<std::mutex> m_wait(_mutex_quit);
            //ZDBG("waiting for %d ms\n",ms_next_wait);
            if(_cond_quit.wait_for(m_wait,std::chrono::milliseconds (ms_next_wait)) !=std::cv_status::timeout) {
                _running=false;
                return;
            }
            ms_elapsed=ms_next_wait;

        }
    }
    catch (std::exception &e)
    {
        Z_ERROR_MSG(zs_internal_error,"Timer Service Thread exception: %s",e.what());
        std::cout << "Type:    " << typeid(e).name() << "\n";
        _running=false;
    }


}