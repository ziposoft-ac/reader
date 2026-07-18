//
// Created by ac on 7/15/26.
//

#include "Battery.h"
#include "../io/i2c.h"
#include "../root.h"


ZMETA(Battery) {
    ZACT(init);
    ZACT(dump);
    ZACT(start);
    ZACT(stop);
    ZACT(shutdown);
    ZPROP(_poll_interval);
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
    if (new_status != _status) {
        _status = new_status;

        switch (_status) {
            case batt_charge_status_charged: {
                ZDBG("batt_charge_status_charged\n");

                root.gpio.ledGreen.off();
                root.gpio.ledRed.on();
            }
            break;

            case batt_charge_status_discharging: {
                ZDBG("batt_charge_status_discharging\n");
                root.gpio.ledGreen.on();
                root.gpio.ledRed.off();
                root.beeper.pushBeeps({{1500, 50}, {1000, 50}, {500, 50}});
            }
            break;
            case batt_charge_status_charging: {
                ZDBG("batt_charge_status_charging\n");

                root.beeper.pushBeeps({{500, 50}, {1000, 50}, {1500, 50}});

                root.gpio.ledGreen.off();
                root.gpio.ledRed.on();
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

    _timer->start(100);
    return zs_ok;
}

z_status Battery::stop() {
    if (init()) return zs_io_error;
    _timer->stop();

    return zs_ok;
}

double get_current(U16 reg) {
    return 0;
}

z_status Battery::read() {
    if (init()) return zs_io_error;
    U16 batt_volt_reg = 0;
    U16 input_volt_reg = 0;
    U16 batt_current_reg = 0;
    U16 input_current_reg = 0;
    i2c_read_word(_i2c_fd, slave_address, INA3221_REG_CH3_I, &batt_current_reg);
    i2c_read_word(_i2c_fd, slave_address, INA3221_REG_CH1_I, &input_current_reg);
    i2c_read_word(_i2c_fd, slave_address, INA3221_REG_CH3_V, &batt_volt_reg);
    i2c_read_word(_i2c_fd, slave_address, INA3221_REG_CH1_V, &input_volt_reg);
    _batt_volt = (double) batt_volt_reg / 1000;
    _input_volt = (double) input_volt_reg / 1000;

    _batt_current = (int) static_cast<short>(batt_current_reg);
    _input_current = (int) static_cast<short>(input_current_reg);
    ZDBG("batt voltage=%04x, %0.2lf\n", batt_volt_reg, _batt_volt);
    ZDBG("input voltage=%04x %0.2lf\n", input_volt_reg, _input_volt);
    ZDBG("batt current=%04x, %0.2lf\n", batt_current_reg, _batt_current);
    ZDBG("input current=%04x %0.2lf\n", input_current_reg, _input_current);

    return zs_ok;
}

z_status Battery::dump() {
    if (read()) return zs_io_error;
    printf("batt voltage=%0.2lf\n", _batt_volt);
    printf("input voltage=%0.2lf\n", _input_volt);
    printf("batt current=%0.2lf\n", _batt_current);
    printf("input current=%0.2lf\n", _input_current);

    return zs_ok;
}

z_status Battery::init() {
    if (_i2c_fd > 0)
        return zs_ok;
    _i2c_fd = i2c_init();
    if (_i2c_fd < 1) {
        _i2c_fd = 0;
        return zs_access_denied;
    }
    i2c_write_word(_i2c_fd, slave_address, 0, INA3221_RESET);
    i2c_write_word(_i2c_fd, slave_address, 0, INA3221_CONFIG);
    if (!_timer)
        _timer = gTimerService.create_timer_t(this, &Battery::timer_callback, 0);
    return zs_ok;
}

z_status Battery::shutdown() {
    if (_i2c_fd) {
        i2c_close(_i2c_fd);
        _i2c_fd = 0;
    }
    return zs_ok;
}
