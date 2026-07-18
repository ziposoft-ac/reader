//
// Created by ac on 11/12/20.
//

#ifndef ZIPOSOFT_GPIO_H
#define ZIPOSOFT_GPIO_H
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



class Gpio;
class GpioPin
{
    friend z_factory_t<GpioPin>;



    struct gpiod_line_request *_request=nullptr;
    gpiod_line_direction _dir=GPIOD_LINE_DIRECTION_INPUT;

protected:
    // only support one gpio chip for now
    //Gpio *_chip=0;
    Timer* _timer=0;

    z_string _name;
    virtual void _off();
    virtual void _on();
    virtual int timer_callback(void*);
    int _state=0;
    bool _output=true;
public:
    unsigned int _pin=0;

    GpioPin(int pin=0);
    virtual ~GpioPin(){}


    z_status setInput();
    z_status setOutputState(bool state);
    z_status setOutput();
    z_status show();
    z_status toggle();
    z_status off();
    z_status on();
    virtual void init(Gpio* chip,ctext name);
    virtual void shutdown();
    z_status json_config_get(z_json_stream &js);

};

class GpioPinLed : public GpioPin
{
    friend z_factory_t<GpioPinLed>;

protected:
    virtual int timer_callback(void*);

    int _delay_on=100;
    int _delay_off=100;
    int _flashCount=0;
    int _flashCountMax=10;

public:
    GpioPinLed(int pin=0) : GpioPin(pin){}
    virtual ~GpioPinLed(){}
    z_status flash(int count);
    z_status toggling_start();
    virtual void init(Gpio* chip,ctext name);

};

typedef std::pair<int,int> Beep;

class GpioBeep : public GpioPin
{
    friend z_factory_t<GpioBeep>;
    z_safe_queue<Beep> _queue;

public:

protected:
    int _next_time_off=0;


    int timer_callback(void*) override;
    virtual void _off() override;
    virtual void _on() ;
public:
    bool _quiet=false;
    bool _enabled=false;
    void pushBeeps(std::initializer_list<Beep> const beeps);
    GpioBeep(int pin=0) : GpioPin(pin){}
    virtual ~GpioBeep(){}
    virtual void init(Gpio* chip);

    virtual void shutdown() {
        off();
        GpioPin::shutdown();
    };
    z_status beep(int duration);

};
class Gpio {
    bool _initialized=false;
    bool _simulate=false;

    Timer* _timer=0;
    int timer_callback(void*);

public:
    //z_obj_vector<GpioPin, false> _pins;

    Gpio();
    virtual ~Gpio();
    enum Led{
        GREEN=26,
        RED=20,
        YELLOW=21,
    };
    //GpioPinLed ledRed=RED;
    //GpioPinLed ledBlue=BLUE;
    //GpioPinLed ledGreen=GREEN;
    //GpioPinLed g2=2;

    // DO NOT USE PINS 2 and 3


    GpioPinLed ledRed=20;
    GpioPinLed ledGreen=26;
    GpioPinLed ledYellow=21;
    GpioPinLed readBeep=23;
    //GpioPinLed ledYellow=YELLOW;
    //GpioBeep beeper=2; - TIMER 3A, confilicts with I2C
    GpioBeep beeper=5;
    const int led_gpio[4]={GREEN,YELLOW,RED};
    bool initialize();
    bool shutdown();
    z_status set(int gpio,int val);
    z_status dump();
    z_status dump_pins();
    //z_status beep();
    z_status lightShow();
	struct gpiod_chip *_chip=0;
    z_status json_config_get(z_json_stream &js);
    z_status led_json_config_set(z_json_obj &jo);


    struct gpiod_line_request* getPinRequest(gpiod_line_direction dir, unsigned int pin);
    int setPinDirection(gpiod_line_direction dir, unsigned int pin);
    int setPinOutput(unsigned int pin,int val);
    int reconfigure_line(struct gpiod_line_request *request,unsigned int offset,gpiod_line_direction dir,enum gpiod_line_value value=GPIOD_LINE_VALUE_INACTIVE);
    int enableInterrupt(int pin);
   // z_status addPin(int num);

    //z_obj_map<GpioPin, false> _led_map;
    z_obj_map<GpioPinLed, false> _led_map;
};

// currently only support 1 gpio chip
// This is ugly, but I gotta get this shit done
extern Gpio gGpio;


#endif //ZIPOSOFT_GPIO_H
