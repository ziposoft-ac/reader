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
enum timer_operation_t {
    timer_start_reset,
    timer_start_if_earlier,
    timer_start_if_not_running,
    timer_oper_stop,
    timer_oper_delete

};
class Timer {
    friend TimerService;
protected:
    z_string _name;
    bool _running=false;
    U64 _ts_expire=0;
    TimerService* _service;
    TimerCallback _user_callback;
    void* _user_context;
    virtual int invoke_callback();

    U64 _update();
public:
    Timer(ctext name,TimerService *service, TimerCallback callback, void* user_context);
    virtual ~Timer();
    //void start();
    void stop();
    void start_ms_if_not_running(U32 ms);
    void start_ms_if_sooner(U32 ms);
    void start_ms_reset(U32 ms);

    void start_ts_if_not_running(const z_time& ts);
    void start_ts_if_sooner(const z_time& ts);
    void start_ts_reset(const z_time& ts);
    void restart(int ms);
    U64 get_timout_ts() const
    { return _ts_expire; }
    bool is_running() const  { return _running; }
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

    Timer_t(ctext name,TimerService *service,T* object, member_callback callback, void* user_context) : Timer(name,service,0,user_context)
    {
        _object=object;
        _member_callback=callback;
    }



};

struct timer_start_work_t {
    Timer *t;
    z_time ts;
    timer_operation_t oper;

};
class TimerService {
    friend Timer;
    std::mutex _mutex_sync;
    std::thread _thread_handle;
    std::thread::id _thread_id=std::thread::id();
    std::set<Timer*> _timers;
    bool _timer_service_running=false;
    bool _flag_reprocess_timers=false;
    std::thread _thread_process;
    void process_thread();
    std::mutex _mutex_stop_wait;
    std::condition_variable _cond_stop_wait;
    U64 _ts_next_expire=0;
    bool update_timers();
    z_safe_queue<timer_start_work_t> _work_queue;

    /*
     * Dont call this directly.
     */
    inline z_status _timer_start_ms(Timer* timer,U32 ms,timer_operation_t cond);
    z_status _timer_start_ts(Timer* timer,const z_time& ts,timer_operation_t cond);

    bool _process_work(timer_start_work_t work);
    z_status _timer_start_ts_orig(Timer* timer,const z_time& ts,timer_operation_t cond);
    void timer_stop(Timer* timer);
public:
    TimerService();
    ~TimerService();


    z_status init();
    z_status shutdown();
    z_status test();

    Timer* createTimer(ctext name,TimerCallback callback, void* user_data, int start = 0);
    bool remove_timer(Timer* timer);
    template <class  T>  Timer* create_timer_t(ctext name,T* object, int (T::*callback)(void*) , void* user_context=0, int start = 0)
    {
        Timer_t<T>* timer=z_new Timer_t<T>(name,this,object,callback,user_context);
        _timers.insert(timer);
        if(start)
            timer->start_ms_reset(start);
        return timer;
    }

};
#define CREATE_TIMER(_X_) gTimerService.create_timer_t(#_X_,this,&_X_ ,0,0 );
#define CREATE_TIMER_EX(_X_,ctx,start) gTimerService.create_timer_t(#_X_,this,&_X_ ,ctx,start );

// Need for more than one timer service?
extern TimerService gTimerService;

#endif //ZIPOSOFT_TIMERS_H
