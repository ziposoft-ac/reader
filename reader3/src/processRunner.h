//
// Created by ac on 11/12/20.
//

#ifndef PROCESS_RUNNER_H
#define PROCESS_RUNNER_H
#include "pch.h"

#include "timers.h"
#include "rfid.h"



class ProcessRunner : public RfidReadConsumer{
    friend z_factory_t<ProcessRunner>;
    bool _open=false;
    z_time _t_started;
    int timer_callback(void*);
    Timer* _timer=0;

    z_obj_map<RfidTag> _tags;
    //PROPS
    std::mutex _mutex;

public:
    ProcessRunner();

    int _min_split_time = 5;
    int _missing_count_time = 1000;

    virtual z_status stop();

    virtual z_status start();


    virtual bool callbackRead(RfidRead* r);
    virtual bool callbackQueueEmpty();

    bool _print_reads=false;

};


ZMETA_DECL(ProcessRunner) {

    ZACT(start);
    ZACT(stop);
    ZPROP(_print_reads);
   // ZPROP(_open);
    ZPROP(_min_split_time);
    ZPROP(_missing_count_time);

}

#endif //ZIPOSOFT_APP_H