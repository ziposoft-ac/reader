//
// Created by ac on 11/12/20.
//

#include "gpio.h"
#include "root.h"
#include <gpiod.h>

const int RED=03;
const int BLUE=22;
const int GREEN=17;
const int YELLOW=27;

const char *chipname = "/dev/gpiochip0";

const int led_gpio[]={RED,GREEN,YELLOW,BLUE};
const int num_leds=sizeof(led_gpio)/sizeof(int);
ZMETA(GpioPin)
{
    ZPROP(_delay_on);
    ZPROP(_state);
    ZPROP(_output);
    ZPROP(_delay_off);
    ZPROP(_flashCountMax);
    ZPROP_F(_pin,ZFF_READ_ONLY);
    ZPROP_F(_flashCount,ZFF_READ_ONLY);
    ZCMD(flash, ZFF_CMD_DEF, "flash",
         ZPRM(int, count, 1, "count", ZFF_PARAM)
         );
    ZACT(off);
    ZACT(toggle);
    ZACT(setOutput);
    ZACT(setInput);
    ZACT(show);
    ZACT(on);
};
ZMETA(GpioPinLed)
{
    ZBASE(GpioPin);

};
ZMETA(GpioPinBuzzer)
{
    ZBASE(GpioPin);
    ZACT(toneRise);
    ZPROP(_quiet);
    ZCMD(beep, ZFF_CMD_DEF, "beep",
         ZPRM(int, freq, 2000, "freq", ZFF_PARAM),
         ZPRM(int, duration, 100, "duration", ZFF_PARAM)
         );
};
GpioPin::GpioPin(int pin)
{
    _pin=pin;
}
int GpioPin::timer_callback(void *)
{
    if(!_flashCount)
        return 0;

    if(_state)
    {
        _state=false;
        _off();
        _flashCount--;
        return _delay_off;

    }
    else
    {
        _state=true;
        _on();
        return _delay_on;

    }
}

void GpioPin::init(Gpio* chip)
{
    Z_ASSERT(!_timer);

    _flashCount=0;
    _chip=chip;
    if(!_timer)
        _timer=root.timerService.create_timer_t(this,&GpioPin::timer_callback,0    );

    _request=chip->getPinRequest((_output?GPIOD_LINE_DIRECTION_OUTPUT: GPIOD_LINE_DIRECTION_INPUT),_pin);
    if (_output) {
        ZLOG("Setting GPIO#%d to output:%d\n",this->_pin,_state);
        gpiod_line_request_set_value(_request, _pin, (gpiod_line_value)_state);

    }
    //gpioSetMode(_pin,1);
    //gpioWrite(_pin,0);
}
void GpioPin::shutdown()
{
    if(_timer)
    {
        _timer->stop();
    }
    setInput();
    if(_request) {
        gpiod_line_request_release(_request);
        _request=0;
    }
}
void GpioPin::_off()
{
    _state=0;

    //gpioWrite(_pin,0);
    gpiod_line_request_set_value(_request, _pin, (gpiod_line_value)_state);

}
z_status GpioPin::toggle()
{
    if(!root.gpio.initialize())
        return zs_io_error;
    _timer->stop();
   _flashCount=100000;
   _timer->start(1,true);

    return zs_ok;
}
z_status GpioPin::flash(int count)
{
    if(!root.gpio.initialize())
        return zs_io_error;
    if(!_flashCount)
        _timer->start(1,false);
    _flashCount+=count;
    if(_flashCount>_flashCountMax)
        _flashCount=_flashCountMax;
    return zs_ok;
}
z_status GpioPin::setInput()
{
    _dir=GPIOD_LINE_DIRECTION_INPUT;
    _output=false;
    _chip->reconfigure_line(_request,_pin,_dir);
    //gpioWrite(_pin,0);
    return zs_ok;
}
z_status GpioPin::setOutput()
{
    _output=true;

    //gpioWrite(_pin,0);
    _dir=GPIOD_LINE_DIRECTION_OUTPUT;
    _chip->reconfigure_line(_request,_pin,_dir);
    gpiod_line_request_set_value(_request, _pin, (gpiod_line_value)_state);

    return zs_ok;
}
void GpioPin::_on()
{
    //gpioWrite(_pin,1);
    _state=1;
    gpiod_line_request_set_value(_request, _pin, (gpiod_line_value)_state);

}
z_status GpioPin::show()
{
    struct gpiod_line_info *info=gpiod_chip_get_line_info(_chip->_chip,_pin);
    if(!info) {
        return Z_ERROR_MSG(zs_io_error,"gpiod_chip_get_line_info failed");
    }
    auto ed=gpiod_line_info_get_direction(info);
    int value = gpiod_line_request_get_value(_request, _pin);
    if (value < 0) {
        perror("gpiod_line_get_value");
    } else {
        printf("GPIO#%d %s level: %d\n",_pin,(ed==GPIOD_LINE_DIRECTION_INPUT?"INPUT":"OUTPUT"), value);
    }


    return zs_ok;
}
z_status GpioPin::off()
{
    if(!root.gpio.initialize())
        return zs_io_error;
    setOutput();

    _timer->stop();

    _off();
    return zs_ok;
}
z_status GpioPin::on()
{
    if(!root.gpio.initialize())
        return zs_io_error;
    setOutput();
    _timer->stop();
    _on();
    return zs_ok;
}
#define PWM_PATH "/sys/class/pwm/pwmchip0/pwm0/" // Adjust for pwmchip1/pwm1 if needed
#define PWM_CHIP  "/sys/class/pwm/pwmchip0/export"




int syswr(ctext filename,int i) {
    z_string s=i;
    FILE* fd = fopen(filename, "wb");
    if (!fd) {
        perror("Failed to open export file");
        return 1;
    }
    fwrite(s.c_str(), s.size(),1,fd); // Export PWM channel 0
    //ZLOG("Writing %s:%s\n",filename,s.c_str());
    fclose(fd);
    return 0;
}
int setPwmFreq(int freq) {
    if (freq) {
        U64 period=1000000000/freq;
        syswr(PWM_PATH "period",period);
        syswr(PWM_PATH "duty_cycle",500000);
        syswr(PWM_PATH "enable",1);
    }
    else {
        syswr(PWM_PATH "enable",0);

    }
    return 0;

}

void GpioPinBuzzer::init(Gpio* chip)
{
    Z_ASSERT(!_timer);

    _chip=chip;
    if(!_timer)
        _timer=root.timerService.create_timer_t(this,&GpioPinBuzzer::timer_callback,0    );
    syswr(PWM_CHIP,0);

}

void GpioPinBuzzer::_off()
{

   setPwmFreq(0);
}
void GpioPinBuzzer::_on()
{
    if(_quiet) return;
        setPwmFreq(50000);
}
int GpioPinBuzzer::timer_callback(void *)
{
    Beep beep;
    int delay=0;
    int freq=0;
    if(_queue.pop(beep))
    {
        delay=beep.second;
        if(delay>2000)
        {
            Z_WARN_MSG(zs_bad_parameter,"Buzzer Delay Too Long");
            delay=100;
        }
        freq=beep.first;
        if(!delay)
            delay=1;
    }
    if(!_quiet) {
        setPwmFreq(freq);

    }
    return delay;
}

void GpioPinBuzzer::beepDiminishing(Beep beep)
{
#ifdef NOGPIO
    return;
#endif
    if (!_timer)
        return;

    int count=_queue.get_count();
    if(count>10)
        return;
    if(count)
        beep.second=beep.second-(beep.second/10)*count;

    if(beep.second<10)
        beep.second=10;
    _queue.push( beep);
    beep.first=0;
    _queue.push( beep);
    _timer->start(1,false);

}
z_status GpioPinBuzzer::beep(int freq, int duration) {

    if(!_chip->initialize())
        return zs_io_error;
    pushBeeps({{freq,duration}});
    return zs_ok;
}

void GpioPinBuzzer::pushBeeps(std::initializer_list<Beep> const beeps)
{
    if (!_timer) {
        Z_ERROR_MSG(zs_not_open,"Buzzer not initialized");
        return;
    }
#ifdef NOGPIO
    return;
#endif
    if(_queue.get_count()>10)
        return;
    for(auto i : beeps)
    {
        _queue.push(i);
    }
    _timer->start(1,false);
}

z_status GpioPinBuzzer::toneRise()
{
    pushBeeps({{500,20},{800,20},{1100,20}});
    return zs_ok;
}
ZMETA(Gpio)
{

    ZOBJ(g2);
    ZOBJ(g3);
    ZOBJ(g5);
    ZOBJ(g6);
    ZOBJ(g17);
    ZOBJ(g22);
    ZOBJ(g23);
    ZOBJ(g24);
    /*
    ZOBJ(ledGreen);
    ZOBJ(ledBlue);
    ZOBJ(ledYellow);

    */
    ZOBJ(buzzer);

    ZCMD(set, ZFF_CMD_DEF, "set",
         ZPRM(int, gpio, 0, "gpio", ZFF_PARAM),
         ZPRM(int, val, 0, "val", ZFF_PARAM)
         );
    ZCMD(addPin, ZFF_CMD_DEF, "addPin",
         ZPRM(int, num, 0, "num", ZFF_PARAM)
         );
    ZACT(dump);
    ZACT(takeOnMe);
    ZACT(takeOnMePush);
    ZACT(beep);
    ZACT(lightShow);
    //ZMAP(_pin_map);
    //ZVECT(_pins);
};
z_status Gpio::addPin(int num)
{
    auto pin=new GpioPin(num);
    z_string name="pin"+num;
    _pins.push_back(pin);
    _pin_map.add(name,pin);
    return zs_ok;
}
bool Gpio::shutdown()
{
    if(!_open) return false;

    setPwmFreq(0);

    for(auto pin : _pins)
    {
        pin->shutdown();
    }

    return false;
}


bool Gpio::initialize() {
#ifdef NOGPIO
    return false;
#endif
    if(_initialized) return _open;
    _initialized=true;
    if(!_timer)
        _timer=root.timerService.create_timer_t(this,&Gpio::timer_callback,0    );

    _chip = gpiod_chip_open(chipname);

    if(_chip==nullptr)
    {
        Z_ERROR_MSG(zs_io_error,"gpiod_chip_open failed:%s",strerror(errno));

        _open= false;
        return false;
    }
    _open=true;
    for(auto pin : _pins)
    {
        pin->init(this);
    }
    buzzer.init(this);
    return _open;
}
z_status Gpio::set(int gpio, int val) {
    if(!initialize())
        return zs_io_error;
    zout.format_append("setting gpio %d: %d\n",gpio,val);
    setPinOutput(gpio,val);
    //gpioWrite(gpio,val);
    return zs_ok;
}
z_status Gpio::dump() {
    if(!initialize())
        return zs_io_error;
    int i;
    for ( i=0; i<54; i++)
    {
        //printf("gpio=%d  mode=%d level=%d\n",    i,  gpioGetMode(i), gpioRead(i));
    }
    return zs_ok;
}
z_status Gpio::beep()
{
    if(!initialize())
        return zs_io_error;
    buzzer.pushBeeps({{500,50}});
    //root.gpio.buzzer.pushBeeps({{2000,500}});

    return zs_ok;
}
z_status Gpio::lightShow()
{
    if(!initialize())
        return zs_io_error;

    _timer->start(100,true);
    return zs_ok;
}
int Gpio::timer_callback(void *)
{
    static int index=0;
    int i;
    for(i=0;i<num_leds;i++)
    {
        //int head=(index==3?0:index+1);
        //int val=(i==index)||(i==head);
        int val=(i==index);
        //gpioWrite(led_gpio[i],val);
    }
    if(++index>=num_leds)
        index=0;
    return 100;
}
Gpio::Gpio()
{
/*
    _pins.push_back(&ledRed);
    _pins.push_back(&ledGreen);
    _pins.push_back(&ledBlue);
    _pins.push_back(&ledYellow);
*/
    _pins.push_back(&g2);
    _pins.push_back(&g3);
    _pins.push_back(&g5);
    _pins.push_back(&g6);
    _pins.push_back(&g23);
    _pins.push_back(&g24);
    _pins.push_back(&g22);
    _pins.push_back(&g17);

}
Gpio::~Gpio()
{

    if (_chip)
	    gpiod_chip_close(_chip);

}

int Gpio::setPinDirection(gpiod_line_direction dir, unsigned int pin) {
    struct gpiod_line_request *request = getPinRequest(dir,pin);
    if (request ) {
        gpiod_line_request_release(request);
        return 0;
    }
    return -1;
}

int Gpio::reconfigure_line(struct gpiod_line_request *request,unsigned int offset,
    gpiod_line_direction dir,enum gpiod_line_value value)
{
    struct gpiod_line_settings *settings;
    struct gpiod_line_config *line_cfg;
    int ret = -1;

    settings = gpiod_line_settings_new();
    if (!settings)
        return -1;

    gpiod_line_settings_set_direction(settings,dir);
    if (dir==GPIOD_LINE_DIRECTION_OUTPUT) {
    gpiod_line_settings_set_output_value(settings, value);

    }

    line_cfg = gpiod_line_config_new();
    if (!line_cfg)
        goto free_settings;

    ret = gpiod_line_config_add_line_settings(line_cfg, &offset, 1,
                          settings);
    if (ret)
        goto free_line_config;

    ret = gpiod_line_request_reconfigure_lines(request, line_cfg);

    free_line_config:
        gpiod_line_config_free(line_cfg);

    free_settings:
        gpiod_line_settings_free(settings);

    return ret;
}
struct gpiod_line_request* Gpio::getPinRequest(gpiod_line_direction dir, unsigned int pin)
{
    struct gpiod_line_settings *settings;
    struct gpiod_line_config *line_cfg;
	struct gpiod_line_request *request = NULL;

    settings = gpiod_line_settings_new();

    gpiod_line_settings_set_direction(settings, dir);
    line_cfg = gpiod_line_config_new();
    int ret = gpiod_line_config_add_line_settings(line_cfg, &pin, 1, settings);
    if (ret) {
        Z_ERROR_MSG(zs_io_error,"gpiod_line_config_add_line_settings failed");
    }
    else {
        request = gpiod_chip_request_lines(_chip, 0, line_cfg);
        if (!request) {
            Z_ERROR_MSG(zs_io_error,"gpiod_chip_request_lines failed");
        }
        else {

        }
    }

    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    return request;

}
int Gpio::setPinOutput( unsigned int pin,int value) {
    struct gpiod_line_request *request = getPinRequest(GPIOD_LINE_DIRECTION_OUTPUT,pin);
    if (request ) {
	    gpiod_line_request_set_value(request, pin, (gpiod_line_value)value);

        gpiod_line_request_release(request);
        return 0;
    }
    return -1;

}