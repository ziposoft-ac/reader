//
// Created by ac on 11/12/20.
//

#ifndef ZIPOSOFT_GPIO_H
#define ZIPOSOFT_GPIO_H
#include "pch.h"
#include "util/timers.h"

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
    virtual z_status _off();
    virtual z_status _on();
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
    virtual z_status init(Gpio* chip,ctext name);
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
    //int _flashCount=0;
    int _toogleCount=0;
    bool _steady_state=false;
    int _flashCountMax=10;

public:
    GpioPinLed() : GpioPin(){}
    virtual ~GpioPinLed(){}
    z_status flash(int count);
    z_status toggling_start();
    virtual z_status init(Gpio* chip,ctext name);
    z_status off();
    z_status on();
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
    virtual z_status _off() override;
    virtual z_status _on() ;
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


/*
 *

Orange PI Zero 2W

GPIO chip =1
PIN#40 = PI3, GPIO21, gpio offset=259

http://www.orangepi.org/orangepiwiki/index.php/Orange_Pi_Zero_2W#How_to_set_the_pull-down_resistor_of_40_Pin_GPIO_port

*/
class Gpio {
    friend z_factory_t<Gpio>;
    friend GpioPin;

    bool _initialized=false;
    bool _simulate=false;
    int _chip_number=0;

    Timer* _timer=0;
    int timer_callback(void*);
    Gpio();

	gpiod_chip *_chip=0;

public:

    virtual ~Gpio();

    //z_obj_vector<GpioPin, false> _pins;
    static Gpio& getInstance() {
        // Guaranteed to be destroyed and initialized thread-safely since C++11
        static Gpio instance;
        return instance;
    }
    gpiod_chip* getDriverHandle() {
        return _chip;
    }
    enum Led{
        GREEN=26,
        RED=20,
        YELLOW=21,
    };
    /*

     Octopi

    GpioPinLed ledRed=20;
    GpioPinLed ledGreen=26;
    GpioPinLed ledYellow=21;
    GpioPinLed readBeep=23;
    */
    GpioPinLed ledRed;
    GpioPinLed ledGreen;
    GpioPinLed ledYellow;
    GpioPinLed readBeep;

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
extern Gpio& gGpio;


#endif //ZIPOSOFT_GPIO_H
