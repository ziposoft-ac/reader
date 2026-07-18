//
// Created by ac on 11/2/20.
//

#ifndef ZIPOSOFT_TIMERS_H
#define ZIPOSOFT_TIMERS_H

#include "pch.h"

class TimerService;
/*
 * Timer callback
 * Return value is the number of milliseconds to run again
 * return 0 to stop the timer.
  */
typedef int (*TimerCallback)(void* data);
class Timer {
    friend TimerService;
protected:
    bool _running=false;
    int _interval=0;
    U64 _ts_expire=0;
    TimerService* _service;
    TimerCallback _user_callback;
    void* _user_context;
    virtual int invoke_callback();

    U64 update();
public:
    Timer(TimerService *service, TimerCallback callback, void* user_context);
    virtual ~Timer();
    void start();
    void stop();
    void start(int ms,bool reset=true);
    void restart(int ms);
    void set_minimum_ts_expire(U64 ts) {
        if (ts<_ts_expire)
            _ts_expire=ts;
    }
    bool _debug=false;

};

template <class T> class  Timer_t : public Timer
{
    friend TimerService;
    virtual int invoke_callback()
    {
        return  (_object->*_member_callback)(_user_context);
    }
    T* _object=0;
public:
    typedef int (T::*member_callback)(void* data);
    member_callback _member_callback;

    Timer_t(TimerService *service,T* object, member_callback callback, void* user_context) : Timer(service,0,user_context)
    {
        _object=object;
        _member_callback=callback;
    }



};


class TimerService {
    friend Timer;
    std::mutex _mutex_sync;
    std::thread _thread_handle;
    std::thread::id _thread_id=std::thread::id();
    std::set<Timer*> _timers;
    bool _running=false;
    bool _flag_reprocess_timers=false;
    std::thread _thread_process;
    void process_thread();
    std::mutex _mutex_stop_wait;
    std::condition_variable _cond_stop_wait;
    U64 _ts_next_expire=0;
    bool update_timers();

    /*
     * Dont call this directly.
     */
    z_status timer_start(Timer* timer,int ms,bool reset=false);
    void timer_stop(Timer* timer);
public:
    TimerService();
    ~TimerService();


    z_status stop();
    z_status test();

    Timer* createTimer(TimerCallback callback, void* user_data, int start = 0);
    bool remove_timer(Timer* timer);
    template <class  T>  Timer* create_timer_t(T* object, int (T::*callback)(void*) , void* user_context, int start = 0)
    {
        Timer_t<T>* timer=z_new Timer_t<T>(this,object,callback,user_context);
        _timers.insert(timer);
        if(start)
            timer->start(start,true);
        return timer;
    }

};

// Need for more than one timer service?
extern TimerService gTimerService;

#endif //ZIPOSOFT_TIMERS_H
