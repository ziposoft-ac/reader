//
// Created by ac on 7/15/26.
//

#ifndef ZIPOSOFT_BATTERY_H
#define ZIPOSOFT_BATTERY_H
#include "pch.h"
#include "../util/timers.h"



enum BatteryChargeStatus {
    batt_charge_status_invalid,
    batt_charge_status_charging,
    batt_charge_status_discharging,
    batt_charge_status_charged,
} ;
class Battery {
    friend z_factory_t<Battery>;

    double _batt_volt=0;
    double _input_volt=0;
    double _batt_current=0;
    double _input_current=0;
    double _batt_current_prev=0;

    BatteryChargeStatus _status=batt_charge_status_invalid;

    int _i2c_fd =0;

    bool _init=false;
    Timer* _timer=0;
    virtual int timer_callback(void*);


    int _poll_interval=1000;

    z_status init();
    z_status shutdown();
    z_status start();
    z_status read();
    z_status stop();
    z_status dump();

};



#endif //ZIPOSOFT_BATTERY_H
