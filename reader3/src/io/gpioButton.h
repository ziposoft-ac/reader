//
// Created by ac on 7/10/26.
//

#ifndef ZIPOSOFT_GPIOBUTTON_H
#define ZIPOSOFT_GPIOBUTTON_H
#include <sys/poll.h>

#include "pch.h"
#include "../util/timers.h"
#ifndef NO_GPIO
#include <gpiod.h>
#else

enum gpiod_line_direction {
    GPIOD_LINE_DIRECTION_AS_IS = 1,
    GPIOD_LINE_DIRECTION_INPUT,
    GPIOD_LINE_DIRECTION_OUTPUT,
};
struct gpiod_line_request;
enum gpiod_line_value {
    GPIOD_LINE_VALUE_ERROR = -1,
    GPIOD_LINE_VALUE_INACTIVE = 0,
    GPIOD_LINE_VALUE_ACTIVE = 1,
};
#endif


class gpioButton {
    friend z_factory_t<gpioButton>;
    virtual int timer_callback_debounce(void*);
    virtual int timer_callback_hold_function(void*);
    virtual int timer_callback_counter_function(void*);
    Timer* _timer_debounce=0;
    Timer* _timer_hold_function=0;
    Timer* _timer_counter_function=0;
    std::thread _thread_handle;
    bool _quit=false;
    struct pollfd _pollfds[2];
    int _event_quit=0;
    int _event_line=0;
    int _press_counter=0;
    int _press_counter_window_ms=2000;
    int _hold_count=0;
    bool _running=false;
    virtual void thread() ;
    unsigned int _pin=26;
    unsigned int _debounce_time=100;
	struct gpiod_chip *_chip=0;
    struct gpiod_edge_event_buffer *_buffer=0;
    struct gpiod_line_request *_request=0;;
    struct gpiod_line_config *_line_cfg=0;;
    struct gpiod_line_settings *_settings=0;

    enum gpiod_line_value val_steady;
    enum gpiod_line_value val_candidate;

    void press_counter_start();
    void press_counter_stop();
public:
    z_status start();
    z_status stop();
};


#endif //ZIPOSOFT_GPIOBUTTON_H
