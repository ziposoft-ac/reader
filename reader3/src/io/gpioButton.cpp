//
// Created by ac on 7/10/26.
//

#include "gpioButton.h"
#include "../global.h"

#include <sys/eventfd.h>
ZMETA(gpioButton)
{

    ZPROP(_pin);
    ZPROP(_debounce_time);
    ZPROP(_press_counter_window_ms);

    ZACT(start);
    ZACT(stop);
};

void gpioButton::press_counter_start() {
    _timer_counter_function->start(_press_counter_window_ms,true);

}

void gpioButton::press_counter_stop() {
}

int gpioButton::timer_callback_debounce(void *) {

    val_steady=val_candidate;


    if (val_steady==1) {

        // Button released
        _timer_hold_function->stop();
        if (_hold_count>8) {
            system("sudo /sbin/poweroff");
            process_quit_notify();

        }
        if (!_press_counter) {
            // start a timer to count presses within
            press_counter_start();

        }
        _press_counter++;

    }
    else {
        // Button pressed
        _hold_count=0;
        _timer_hold_function->start(1000,true);
        //root.beeper.pushBeeps({{500,10}});

    }

    return 0;
}
int gpioButton::timer_callback_hold_function(void *) {
    _hold_count++;
    ZDBG("hold count=%d\n",_hold_count);
    if (_hold_count<3) {
        // change to service call
        ZDBG("hold count LEVEL 1\n");

        //root.gpio.ledGreen.flash(1);
        //root.beeper.pushBeeps({{1000,20}});

    }else {
        if (_hold_count<6)
        {
            ZDBG("hold count LEVEL 3\n");

            //root.gpio.ledYellow.flash(1);
            //root.beeper.pushBeeps({{1500,20}});

        }
        else {
            ZDBG("hold count LEVEL 6!!\n");

            //root.gpio.ledRed.flash(1);
            //root.beeper.pushBeeps({{2000,30}});
        }
    }

    press_counter_stop();

    return 1000;
}
int gpioButton::timer_callback_counter_function(void *) {
    // this expires after 2 seconds of pressing

    ZDBG("Press counter=%d\n",_press_counter);
    if (_press_counter==5) {
        //root.beeper.takeOnMePush();
        printf("take on me");
    }
    _press_counter=0;
    return 0;
}
z_status gpioButton::start() {
    if (_running)
        return zs_already_open;
    _quit = false;
    const char *chip_path = "/dev/gpiochip0";

    _chip = gpiod_chip_open(chip_path);
    _settings = gpiod_line_settings_new();
    // Set line direction to INPUT and listen for both RISING and FALLING edges
    gpiod_line_settings_set_direction(_settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(_settings, GPIOD_LINE_EDGE_BOTH);
    gpiod_line_settings_set_bias(_settings, GPIOD_LINE_BIAS_PULL_UP);
    _line_cfg = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(_line_cfg, &_pin, 1, _settings);
    _buffer = gpiod_edge_event_buffer_new(16);

    _request = gpiod_chip_request_lines(_chip, NULL, _line_cfg);
    if (!_request) {
        perror("Line request failed");
        return zs_io_error;
    }
    _event_quit = eventfd(0, EFD_NONBLOCK);
    _event_line=gpiod_line_request_get_fd(_request);
    _pollfds[0]={_event_line,POLLIN,0};
    _pollfds[1]={_event_quit,POLLIN,0};

    _thread_handle = std::thread(&gpioButton::thread, this);
    _running = true;
    if(!_timer_debounce)
        _timer_debounce=gTimerService.create_timer_t(this,&gpioButton::timer_callback_debounce,0    );
    if(!_timer_hold_function)
        _timer_hold_function=gTimerService.create_timer_t(this,&gpioButton::timer_callback_hold_function,0    );
    if(!_timer_counter_function)
        _timer_counter_function=gTimerService.create_timer_t(this,&gpioButton::timer_callback_counter_function,0    );
    return zs_ok;
}

void gpioButton::thread() {
    val_steady=gpiod_line_request_get_value(_request, _pin);

    while (!_quit) {
        if (!_request) {
            break;
        }
        int ret = poll(_pollfds, 2, -1);

        if (ret == -1) {
            fprintf(stderr, "error waiting for edge events: %s\n",
                strerror(errno));
            break;
        }
        if (_pollfds[1].revents) {
            ZDBG(" quiting thread\n");
            _quit = true;
            break;
        }
        ret = gpiod_line_request_read_edge_events(_request, _buffer, 16);



        if (ret < 0) {
            perror("Error reading edge events");
            break;
        }
        enum gpiod_line_value val=gpiod_line_request_get_value(_request, _pin);
        ZDBG("val=%d\r",val);
        if (val!=val_steady) {
            val_candidate=val;
            _timer_debounce->start(_debounce_time,true);
        }
        else {
            _timer_debounce->stop();
        }


        /*
        // Iterate through all intercepted events inside the buffer
        for (int i = 0; i < ret; i++) {
            struct gpiod_edge_event *event = gpiod_edge_event_buffer_get_event(_buffer, i);
            enum gpiod_edge_event_type type = gpiod_edge_event_get_event_type(event);
            printf("Event detected: %s\n",             type == GPIOD_EDGE_EVENT_RISING_EDGE ? "RISING (Pressed)" : "FALLING (Released)");
        }*/
    }
    printf(" gpioButton::thread EXIT");

}

z_status gpioButton::stop() {
    if (!_running)
        return zs_not_open;
    _quit = true;
    if(_timer_debounce)
        _timer_debounce->stop();
    if(_timer_hold_function)
        _timer_hold_function->stop();

    uint64_t value = 1;
    write(_event_quit, &value, sizeof(value)); // Signal the poll_thread


    if (_thread_handle.joinable())
        _thread_handle.join();
    if (_request) {
        ZDBG("Releasing request");
        gpiod_line_request_release(_request);
        _request=0;

    }


    _running = false;

    gpiod_chip_close(_chip);

    if (_line_cfg) {
        gpiod_line_config_free(_line_cfg);
        _line_cfg=0;

    }
    if (_settings) {
        gpiod_line_settings_free(_settings);
        _settings=0;

    }
    if (_buffer) {
        gpiod_edge_event_buffer_free(_buffer);
        _buffer=0;
    }
    return zs_ok;
}
