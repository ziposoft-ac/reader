//
// Created by ac on 11/12/20.
//

#ifndef ZIPOSOFT_GPIO_H
#define ZIPOSOFT_GPIO_H
#include "pch.h"
#include "timers.h"

#include <gpiod.h>


class Gpio;
class GpioPin
{
    struct gpiod_line_request *_request=nullptr;
    gpiod_line_direction _dir=GPIOD_LINE_DIRECTION_INPUT;
protected:
    Gpio *_chip;
    friend z_factory_t<GpioPin>;
    Timer* _timer=0;
    int _delay_on=100;
    int _delay_off=100;
    int _flashCount=0;
    int _flashCountMax=10;
    int _state=0;
    bool _output=false;

    unsigned int _pin=0;
    virtual void _off();
    virtual void _on();
    virtual int timer_callback(void*);

public:
    GpioPin(int pin=0);
    virtual ~GpioPin(){}
    z_status flash(int count);
    z_status toggle();

    z_status setInput();
    z_status setOutput();
    z_status show();
    z_status off();
    z_status on();
    virtual void init(Gpio* chip);
    virtual void shutdown();
};
class GpioPinLed : public GpioPin
{
public:
    GpioPinLed(int pin=0) : GpioPin(pin){}
    virtual ~GpioPinLed(){}

};
class GpioPinBuzzer : public GpioPin
{
public:

    typedef std::pair<int,int> Beep;
private:
    virtual int timer_callback(void*);
    z_safe_queue<Beep> _queue;
protected:
    virtual void _off();
    virtual void _on();
public:
    bool _quiet=false;
    void pushBeeps(std::initializer_list<Beep> const beeps);
    void beepDiminishing(Beep beep);
    GpioPinBuzzer(int pin=0) : GpioPin(pin){}
    virtual ~GpioPinBuzzer(){}
    z_status toneRise();
    virtual void init(Gpio* chip);

    virtual void shutdown(){ off(); };
    z_status beep(int freq, int duration);

};
class Gpio {
    bool _initialized=false;
    bool _open=false;
    bool _simulate=false;

    Timer* _timer=0;
    int timer_callback(void*);

public:
    z_obj_vector<GpioPin, false> _pins;

    Gpio();
    virtual ~Gpio();
    enum Led{
        RED=3,
        BLUE=23,
        GREEN=17,
        YELLOW=27,
    };
    //GpioPinLed ledRed=RED;
    //GpioPinLed ledBlue=BLUE;
    //GpioPinLed ledGreen=GREEN;
    GpioPinLed g2=2;
    GpioPinLed g3=3;
    GpioPinLed g5=5;
    GpioPinLed g6=6;
    GpioPinLed g17=17;
    GpioPinLed g22=22;
    GpioPinLed g23=23;
    GpioPinLed g24=24;
    //GpioPinLed ledYellow=YELLOW;
    GpioPinBuzzer buzzer=18;
    const int led_gpio[4]={RED,GREEN,YELLOW,BLUE};
    bool initialize();
    bool shutdown();
    z_status set(int gpio,int val);
    z_status dump();
    z_status beep();
    z_status takeOnMe();
    z_status takeOnMePush();
    z_status lightShow();
	struct gpiod_chip *_chip=0;


    struct gpiod_line_request* getPinRequest(gpiod_line_direction dir, unsigned int pin);
    int setPinDirection(gpiod_line_direction dir, unsigned int pin);
    int setPinOutput(unsigned int pin,int val);
    int reconfigure_line(struct gpiod_line_request *request,unsigned int offset,gpiod_line_direction dir,enum gpiod_line_value value=GPIOD_LINE_VALUE_INACTIVE);

    z_status addPin(int num);

    z_obj_map<GpioPin, false> _pin_map;
};


#endif //ZIPOSOFT_GPIO_H
