//
// Created by ac on 7/8/26.
//

#include "BeepPwm.h"

#include "root.h"
ZMETA(BeepPwm) {
    ZACT(toneRise);
    ZPROP(_quiet);
    ZPROP(_duty);
    ZPROP(_enabled);
    ZCMD(buzz, ZFF_CMD_DEF, "buzz",
         ZPRM(int, f0, 8000, "freq0", ZFF_PARAM),
         ZPRM(int, d0, 100, "duration0", ZFF_PARAM),
         ZPRM(int, f1, 800, "freq1", ZFF_PARAM),
         ZPRM(int, d1, 100, "duration1", ZFF_PARAM),
         ZPRM(int, f2, 800, "freq2", ZFF_PARAM),
         ZPRM(int, d2, 100, "duration2", ZFF_PARAM)
         );
};

#define PWM_PATH "/sys/class/pwm/pwmchip0/pwm0/" // Adjust for pwmchip1/pwm1 if needed
#define PWM_CHIP  "/sys/class/pwm/pwmchip0/export"


bool pwm_is_init=false;


int syswr(ctext filename,int i) {
    z_string s=i;
    FILE* fd = fopen(filename, "wb");
    if (!fd) {
        return zs_io_error;
        //perror("Failed to open export file");
        return Z_ERROR_MSG(zs_io_error,"PWM Error writing %d to %s\n",i,filename);
    }
    fwrite(s.c_str(), s.size(),1,fd); // Export PWM channel 0
    //ZLOG("Writing %s:%s\n",filename,s.c_str());
    fclose(fd);
    return 0;
}
int setPwmFreq(int freq,int duty_percent) {
    if (!pwm_is_init) {
        if (syswr(PWM_CHIP,0))
            return -1;
        if (syswr(PWM_PATH "enable",0))
            return -1;
        pwm_is_init=true;


    }
    if (freq) {
        U64 period=1000000000/freq;
        U64 duty=period*duty_percent/100;
        if (syswr(PWM_PATH "period",period))
            return -1;
        if (syswr(PWM_PATH "duty_cycle",duty))
            return -1;

        if (syswr(PWM_PATH "enable",1))
            return -1;
    }
    else {
        if (syswr(PWM_PATH "enable",0))
            return -1;


    }
    return 0;

}


int BeepPwm::_off()
{
    return setPwmFreq(0,_duty);
}
int BeepPwm::timer_callback(void *)
{
    Tone beep;
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
    if (_enabled)
        if(!_quiet) {
            setPwmFreq(freq,_duty);

        }
    return delay;
}
void BeepPwm::pushBeeps(std::initializer_list<Tone> const beeps)
{
    if (!_enabled)    return ;

    if (!_timer) {
        Z_ERROR_MSG(zs_not_open,"Buzzer not initialized");
        return;
    }

    if(_queue.get_count()>10)
        return;
    for(auto i : beeps)
    {
        _queue.push(i);
    }
    _timer->start(1,false);
}

int BeepPwm::_on()
{
    if(_quiet) return -1;
    if (_enabled)
        return setPwmFreq(50000,_duty);
    return -1;
}
z_status BeepPwm::toneRise()
{
    if (!_enabled)    return zs_not_open;

    pushBeeps({{500,20},{800,20},{1100,20}});
    return zs_ok;
}

z_status BeepPwm::buzz(int f0,int d0,int f1,int d1,int f2,int d2) {

    if (!_enabled)    return zs_not_open;

    pushBeeps({{f0,d0},{f1,d1},{f2,d2}});
    return zs_ok;
}
z_status BeepPwm::init() {
    if (!_enabled)
        return zs_not_open;
    if (syswr(PWM_CHIP,0)) {
        _exists=false;
        Z_ERROR_MSG(zs_io_error,"PWM init failed, will try again later");
        return zs_io_error;
    }

    if (_off() /*error*/) {
        _exists=false;
        Z_ERROR_MSG(zs_io_error,"PWM does not exists, disabling");
        return zs_io_error;


    }

    _initialized=true;
    if(!_timer)
        _timer=root.timerService.create_timer_t(this,&BeepPwm::timer_callback,0    );
    return zs_ok;
}
void BeepPwm::shutdown()
{
    setPwmFreq(0,0);
}