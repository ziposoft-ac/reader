//
// Created by ac on 11/12/20.
//

#ifndef ZIPOSOFT_GPIO_H
#define ZIPOSOFT_GPIO_H
#include "pch.h"
#include "timers.h"

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
    Gpio *_chip;
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
    z_status toggle();
    virtual void init(Gpio* chip,ctext name);

};
class GpioBeep : public GpioPin
{
    friend z_factory_t<GpioBeep>;

public:

    typedef std::pair<int,int> Beep;
protected:
    int _next_time_off=0;


    virtual int timer_callback(void*);
    z_safe_queue<Beep> _queue;
    virtual void _off();
    virtual void _on();
public:
    bool _quiet=false;
    bool _enabled=false;
    void pushBeeps(std::initializer_list<Beep> const beeps);
    void beepDiminishing(Beep beep);
    GpioBeep(int pin=0) : GpioPin(pin){}
    virtual ~GpioBeep(){}
    virtual void init(Gpio* chip);

    virtual void shutdown(){ off(); };
    z_status beep(int duration);
    z_status up();
    z_status down();

};
class GpioBeepPWM  : public GpioBeep{
protected:
    virtual void _off();
    virtual void _on();
    virtual int timer_callback(void*);

public:
    GpioBeepPWM(int pin=0) : GpioBeep(pin){}
    virtual ~GpioBeepPWM(){}
    z_status buzz(int freq, int duration);
    z_status toneRise();

};
class Gpio {
    bool _initialized=false;
    bool _open=false;
    bool _simulate=false;

    Timer* _timer=0;
    int timer_callback(void*);

public:
    //z_obj_vector<GpioPin, false> _pins;

    Gpio();
    virtual ~Gpio();
    enum Led{
        RED=3,
        BLUE=23,
        GREEN=17,
        YELLOW=22,
    };
    //GpioPinLed ledRed=RED;
    //GpioPinLed ledBlue=BLUE;
    //GpioPinLed ledGreen=GREEN;
    //GpioPinLed g2=2;
    GpioPinLed ledRed=3;
    GpioPinLed g5=5;
    GpioPinLed g6=6;
    GpioPinLed ledGreen=17;
    GpioPinLed ledYellow=22;
    GpioPinLed readBeep=23;
    GpioPinLed g24=24;
    //GpioPinLed ledYellow=YELLOW;
    GpioBeep beeper=2;
    GpioBeepPWM beepPwm=18;
    const int led_gpio[4]={RED,GREEN,YELLOW,BLUE};
    bool initialize();
    bool shutdown();
    z_status set(int gpio,int val);
    z_status dump();
    z_status dump_pins();
    //z_status beep();
    z_status takeOnMe();
    z_status takeOnMePush();
    z_status lightShow();
	struct gpiod_chip *_chip=0;
    z_status json_config_get(z_json_stream &js);
    z_status led_json_config_set(z_json_obj &jo);


    struct gpiod_line_request* getPinRequest(gpiod_line_direction dir, unsigned int pin);
    int setPinDirection(gpiod_line_direction dir, unsigned int pin);
    int setPinOutput(unsigned int pin,int val);
    int reconfigure_line(struct gpiod_line_request *request,unsigned int offset,gpiod_line_direction dir,enum gpiod_line_value value=GPIOD_LINE_VALUE_INACTIVE);

   // z_status addPin(int num);

    //z_obj_map<GpioPin, false> _led_map;
    z_obj_map<GpioPinLed, false> _led_map;
};


#endif //ZIPOSOFT_GPIO_H
