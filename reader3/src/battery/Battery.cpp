//
// Created by ac on 7/15/26.
//

#include "Battery.h"
#include "io/i2c.h"
#include "io/gpio.h"
#include "io/BeepPwm.h"



ZMETA(Battery) {
    ZACT(init);
    ZACT(dump);
    ZACT(start);
    ZACT(stop);
    ZACT(shutdown);
    ZPROP(_poll_interval);
    ZPROP(_debug);
    ZPROP(_shunt_battery);
    ZPROP(_shunt_input);
};




constexpr U16 INA3221_RESET = 0x8000;
constexpr U16 INA3221_EN_CH1 = 0x4000;
constexpr U16 INA3221_EN_CH2 = 0x2000;
constexpr U16 INA3221_EN_CH3 = 0x1000;
constexpr U16 INA3221_REG_CH1_I = 1;
constexpr U16 INA3221_REG_CH1_V = 2;
constexpr U16 INA3221_REG_CH2_I = 3;
constexpr U16 INA3221_REG_CH2_V = 4;
constexpr U16 INA3221_REG_CH3_I = 5;
constexpr U16 INA3221_REG_CH3_V = 6;
constexpr U16 INA3221_V_AVG_1024 = 0x0E00; //1024
constexpr U16 INA3221_V_AVG = 0x0600; //64
constexpr U16 INA3221_V_TIME_MASK = 0x01c0;
constexpr U16 INA3221_V_TIME = 0x0100;

constexpr U16 INA3221_MODE = 0x7;


constexpr U16 INA3221_CONFIG = INA3221_MODE | INA3221_EN_CH1 | INA3221_EN_CH3 | INA3221_V_AVG | INA3221_V_TIME;

constexpr U8 slave_address = 0x40;



int Battery::timer_callback(void *) {
    if (read())
        return 0; // quit if error
    BatteryChargeStatus new_status = batt_charge_status_invalid;

    if (_batt_current < 0) {
        new_status = batt_charge_status_charging;
    } else {
        if (_batt_current > 0.05) {
            new_status = batt_charge_status_discharging;
        } else {
            new_status = batt_charge_status_charged;
        }
    }
    int flash_count=1;
    if (_batt_volt>12) {
        flash_count=(_batt_volt-12)*10 +1;
    }

    ZDBG("batt %d flash count=%d , current=%lf volt=%lf\n",new_status,flash_count,_batt_current,_batt_volt);
    if (new_status==batt_charge_status_charging) {
        gGpio.ledGreen.flash(flash_count);
    }
    if (new_status==batt_charge_status_discharging) {
        gGpio.ledRed.flash(flash_count);
    }

    if (new_status != _status) {
        _status = new_status;

        switch (_status) {
            case batt_charge_status_charged: {
                ZDBG("batt_charge_status_charged\n");

                //gGpio.ledRed.on();
            }
            break;

            case batt_charge_status_discharging: {
                ZDBG("batt_charge_status_discharging\n");
               // gGpio.ledGreen.on();
                //gGpio.ledRed.off();
                gBeepPwm.pushTones({{1500, 50}, {1000, 50}, {500, 50}});
            }
            break;
            case batt_charge_status_charging: {
                ZDBG("batt_charge_status_charging\n");

               // root.beeper.pushBeeps({{500, 50}, {1000, 50}, {1500, 50}});

               // gGpio.ledGreen.off();
               // gGpio.ledRed.on();
            }
            break;
            default:
                break;
        }
    }
    return _poll_interval;
}

z_status Battery::start() {
    if (init()) return zs_io_error;

    _timer->start_ms_reset(100);
    return zs_ok;
}

z_status Battery::stop() {
    if (init()) return zs_io_error;
    _timer->stop();

    return zs_ok;
}

double get_current(U16 reg,double shunt) {

    int intval=(int) static_cast<short>(reg);
    int shifted=intval>>3;
    double volts=shifted*0.00004f;
    double amps=volts/shunt;
    return amps;
}
U16 Battery::read_reg(U16 add) {
    U16 val = 0;
    int retry=2;
    //Sometimes reg reads zero so retry if that happens
    while (retry--) {
        i2c_read_word(_i2c_fd, slave_address, add, &val);
        if (val)
            return val;

    }
    return val;





}

z_status Battery::read() {
#ifdef NOGPIO

    _batt_volt=12+(double)(random()%100)/100;
    _input_volt=13+(double)(random()%100)/100;
    _input_current=2+(double)(random()%100)/100;
    _batt_current=1+(double)(random()%100)/100;
    return zs_ok;
#endif
    if (init()) return zs_io_error;

    U64 ts_start_ns = z_time_get_ticks_ns();
    U64 ts_start_us = z_time_get_ticks_us();
    U64 ts_start_ms = z_time_get_ticks_ms();
    U16 batt_volt_reg = read_reg(INA3221_REG_CH3_V);
    U16 batt_current_reg = read_reg(INA3221_REG_CH3_I);

    U16 input_volt_reg = read_reg(INA3221_REG_CH1_V);
    U16 input_current_reg = read_reg(INA3221_REG_CH1_I);
    _batt_volt = (double) batt_volt_reg / 1000;
    _input_volt = (double) input_volt_reg / 1000;

    _batt_current = get_current(batt_current_reg,_shunt_battery);
    _input_current =  get_current(input_current_reg,_shunt_input);
    if (_debug) {
        U64 elap=z_time_get_ticks_ns()-ts_start_ns;;
        ZDBG("read took:%lld ns\n", elap);
        elap=z_time_get_ticks_us()-ts_start_us;;
        ZDBG("read took:%lld us\n", elap);
        elap=z_time_get_ticks_ms()-ts_start_ms;;
        ZDBG("read took:%lld ms\n", elap);
        ZDBG("batt voltage=%04x, %0.2lf\n", batt_volt_reg, _batt_volt);
        ZDBG("batt current=%04x, %0.2lf\n", batt_current_reg, _batt_current);


        ZDBG("input voltage=%04x %0.2lf\n", input_volt_reg, _input_volt);
        ZDBG("input current=%04x %0.2lf\n", input_current_reg, _input_current);

    }

    return zs_ok;
}

z_status Battery::dump() {
    if (read()) return zs_io_error;
    printf("batt voltage=%0.3lf\n", _batt_volt);
    printf("input voltage=%0.2lf\n", _input_volt);
    printf("batt current=%0.2lf\n", _batt_current);
    printf("input current=%0.2lf\n", _input_current);

    return zs_ok;
}

z_status Battery::json_get(z_json_stream &js) {
    js.set_pretty_print(true);
    js.obj_val_start("battery");

    js.keyval_float("batt_v", _batt_volt);
    js.keyval_float("input_v", _input_volt);
    js.keyval_float("batt_current", _batt_current);
    js.keyval_float("input_current", _input_current);
    js.keyval_int("ts", z_time_get_ticks_ms());
    js.obj_end();

    return zs_ok;
}

z_status Battery::init() {
#ifndef NOGPIO
    if (_i2c_fd > 0)
        return zs_ok;
    _i2c_fd = i2c_init();
    if (_i2c_fd < 1) {
        _i2c_fd = 0;

        return Z_ERROR_MSG(zs_access_denied,"I2C initialization of battery failed\n");
    }
    int res=i2c_write_word(_i2c_fd, slave_address, 0, INA3221_RESET);
    if (res<0) {
        shutdown();
        return Z_ERROR_MSG(zs_io_error,"I2C initialization of battery failed\n");


    }

    res=i2c_write_word(_i2c_fd, slave_address, 0, INA3221_CONFIG);
    if (res<0)
        return zs_io_error;
#endif

    if (!_timer)
        _timer = CREATE_TIMER(Battery::timer_callback);
    return zs_ok;
}

z_status Battery::shutdown() {
    if (_i2c_fd) {
        i2c_close(_i2c_fd);
        _i2c_fd = 0;
    }
    return zs_ok;
}
