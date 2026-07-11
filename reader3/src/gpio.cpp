//
// Created by ac on 11/12/20.
//

#include "gpio.h"
#include "root.h"

const int RED=03;
const int BLUE=22;
const int GREEN=17;
const int YELLOW=27;

const char *chipname = "/dev/gpiochip0";

const int led_gpio[]={RED,GREEN,YELLOW,BLUE};
const int num_leds=sizeof(led_gpio)/sizeof(int);
ZMETA(GpioPin)
{

    ZPROP_F(_pin,ZFF_READ_ONLY);

    ZACT(off);
    ZACT(setOutput);
    ZACT(setInput);
    ZACT(show);
    ZACT(on);
};
ZMETA(GpioPinLed)
{
    ZBASE(GpioPin);
    ZPROP(_delay_on);
    ZPROP(_state);
    ZPROP(_output);
    ZPROP(_delay_off);
    ZPROP(_flashCountMax);
    ZACT(toggling_start);
    ZPROP_F(_flashCount,ZFF_READ_ONLY);
    ZCMD(flash, ZFF_CMD_DEF, "flash",
         ZPRM(int, count, 1, "count", ZFF_PARAM)
         );
};
ZMETA(GpioBeep)
{
    ZBASE(GpioPin);
    ZPROP(_quiet);
    ZPROP(_enabled);

    ZCMD(beep, ZFF_CMD_DEF, "beep",
         ZPRM(int, duration, 100, "duration", ZFF_PARAM)
         );
};

ZMETA(Gpio)
{
    ZOBJ(g5);
    ZOBJ(g6);
    ZOBJ_X(ledGreen,"green",ZFF_PROP_DEF,"Green LED, pin#17");
    //ZOBJ_X(ledRed,"red",ZFF_PROP_DEF,"Red LED, pin#3");
    ZOBJ_X(ledYellow,"yellow",ZFF_PROP_DEF,"Yellow LED, pin#22");
    ZOBJ_X(readBeep,"readBeep",ZFF_PROP_DEF,"Beep on read enable, pin#23");
    ZOBJ(g27);
    ZOBJ(g24);
    ZOBJ(beeper);
    ZCMD(set, ZFF_CMD_DEF, "set",
         ZPRM(int, gpio, 0, "gpio", ZFF_PARAM),
         ZPRM(int, val, 0, "val", ZFF_PARAM)
         );
    /*
    ZCMD(addPin, ZFF_CMD_DEF, "addPin",
         ZPRM(int, num, 0, "num", ZFF_PARAM)
         );*/
    ZACT(dump);
    //ZACT(beep);
    ZACT(lightShow);
    ZACT(dump_pins);
};
/**************************************************************************************
 *
 *   GpioPin
 *
 */
GpioPin::GpioPin(int pin)
{
    _pin=pin;
}
int GpioPin::timer_callback(void *)
{
    return 0;
}

void GpioPin::init(Gpio* chip,ctext name)
{
    Z_ASSERT(!_timer);
    _name=name;
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

z_status GpioPin::toggle()
{
    _state=!_state;
    gpiod_line_request_set_value(_request, _pin, (gpiod_line_value)_state);
    return zs_ok;
}
void GpioPin::_off()
{
    _state=0;

    //gpioWrite(_pin,0);
    gpiod_line_request_set_value(_request, _pin, (gpiod_line_value)_state);

}

z_status GpioPin::setInput()
{
    if (!_request)
        return zs_not_open;
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
    gpiod_line_info *info=gpiod_chip_get_line_info(_chip->_chip,_pin);
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
z_status GpioPin::json_config_get(z_json_stream &js) {
    js.keyval_int("pin",_pin);

    if (!_chip)
        return zs_not_open;
    gpiod_line_info *info=gpiod_chip_get_line_info(_chip->_chip,_pin);
    if(!info) {
        return Z_ERROR_MSG(zs_io_error,"gpiod_chip_get_line_info failed");
    }
    auto ed=gpiod_line_info_get_direction(info);
    int value = gpiod_line_request_get_value(_request, _pin);

    js.keyval("dir",(ed==GPIOD_LINE_DIRECTION_INPUT?"INPUT":"OUTPUT"));
    js.keyval_int("val",value);

    return zs_ok;
}
z_status GpioPin::setOutputState(bool state) {

    return (state?on():off());
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
/**************************************************************************************
 *
 *   GpioPinLed
 *
 */

z_status GpioPinLed::toggling_start()
{
    if(!root.gpio.initialize())
        return zs_io_error;
    _timer->stop();
    _flashCount=100000;
    _timer->start(1,true);

    return zs_ok;
}
z_status GpioPinLed::flash(int count)
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
int GpioPinLed::timer_callback(void *)
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
void GpioPinLed::init(Gpio* chip,ctext name)
{
    _flashCount=0;
    GpioPin::init(chip,name);
}


/**************************************************************************************
 *
 *   GpioBeepPWM
 *
 */







/**************************************************************************************
 *
 *   Gpio
 *
 */

/*
z_status Gpio::addPin(int num)
{
    auto pin=new GpioPin(num);
    z_string name="pin"+num;
    //_pins.push_back(pin);
    //_led_map.add(name,pin);
    return zs_not_implemented;
    return zs_ok;
}
*/
Gpio::Gpio()
{
    auto fact=GET_FACT(Gpio);
    get_child_objs_type(fact,this,_led_map);
}
Gpio::~Gpio()
{
    if (_chip)
        gpiod_chip_close(_chip);
}

bool Gpio::initialize() {

    if(_initialized) return _initialized;

#ifdef NOGPIO
    return false;
#endif
    if(!_timer)
        _timer=root.timerService.create_timer_t(this,&Gpio::timer_callback,0    );




    _chip = gpiod_chip_open(chipname);

    if(_chip==nullptr)
    {
        Z_ERROR_MSG(zs_io_error,"gpiod_chip_open failed:%s %d",strerror(errno),errno);

        _initialized= false;
        return false;
    }
    _initialized=true;
    for(auto p : _led_map)
    {
        p.second->init(this,p.first);
    }
    beeper.init(this);
    return _initialized;
}
bool Gpio::shutdown()
{
    if(!_initialized) return false;


    for(auto p : _led_map)
    {
        p.second->shutdown();
    }
    beeper.shutdown();
    return false;
}
z_status Gpio::led_json_config_set(z_json_obj &jo) {

    ZTF;
    z_json_obj *gpio_cfg=jo.get_child("gpio");
    if (! gpio_cfg)
        return zs_bad_parameter;
    z_json_stream s(ZDBGS,true);
    jo.print(s);
    for(auto pin: _led_map) {
        GpioPinLed* led=pin.second;
        z_json_obj *cfg;

        if (cfg=gpio_cfg->get_child(pin.first))
        {
            I64 i;
            if (cfg->get_int_val("val",i)) {
                led->setOutputState(i);
            }
        }
    }

    return zs_ok;
}



z_status Gpio::json_config_get(z_json_stream &js) {

    js.set_pretty_print(true);
    js.obj_val_start("gpio");

    for(auto pin: _led_map) {
        GpioPinLed* led=pin.second;
        js.obj_val_start(pin.first);
        led->json_config_get(js);

        js.obj_end();
    }
    js.obj_end();

    return zs_ok;
}


z_status Gpio::set(int gpio, int val) {
    if(!initialize())
        return zs_io_error;
    zout.format_append("setting gpio %d: %d\n",gpio,val);
    setPinOutput(gpio,val);
    //gpioWrite(gpio,val);
    return zs_ok;
}
z_status Gpio::dump_pins() {
    auto fact=GET_FACT(Gpio);
    z_obj_map<GpioPinLed,false> map;
    get_child_objs_type(fact,this,map);

    for(auto pin: map) {
        GpioPinLed* led=pin.second;
        zout<< pin.first<<":"<< led->_pin << '\n';
    }

    return zs_ok;
}
z_status Gpio::dump() {
    if(!initialize())
        return zs_io_error;


    return zs_ok;
}
/*
z_status Gpio::beep()
{
    if(!initialize())
        return zs_io_error;
    beepPwm.pushBeeps({{500,50}});
    //root.beeper.pushBeeps({{2000,500}});

    return zs_ok;
}*/
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














/**************************************************************************************
 *
 *   GpioBeep
 *
 */
void GpioBeep::init(Gpio* chip)
{
    Z_ASSERT(!_timer);
    _state=1; //SET OFF
    GpioPin::init(chip,"beep");


}
void GpioBeep::_off()
{
    // Reverse it
    GpioPin::_on();
}
void GpioBeep::_on()
{
    // Reverse it
    GpioPin::_off();
}
typedef std::pair<int,int> Beep;

int GpioBeep::timer_callback(void *)
{
    int delay=0;

    if (_next_time_off) {
        _off();
        delay=_next_time_off;
        _next_time_off=0;
        return delay;
    }


    Beep beep;
    if(_queue.pop(beep))
    {
        delay=beep.first;
        _next_time_off=beep.second;
        if(delay>3000)
        {
            Z_WARN_MSG(zs_bad_parameter,"Beep Delay Too Long");
            delay=100;
        }
        if(!delay)
            delay=1;
    }
    else {
        _off();
        return 0;
    }
    if (_enabled)
        if(!_quiet) {
            _on();
        }
    return delay;
}
z_status GpioBeep::beep( int duration) {

    if (!_enabled)    return zs_not_open;

    if(!_chip->initialize())
        return zs_io_error;
    pushBeeps({{duration,50}});
    return zs_ok;
}

void GpioBeep::pushBeeps(std::initializer_list<Beep> const beeps)
{
    if (!_enabled)    return ;

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

