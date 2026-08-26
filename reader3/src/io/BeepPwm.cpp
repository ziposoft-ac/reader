//
// Created by ac on 7/8/26.
//

#include "BeepPwm.h"



ZMETA(BeepPwm) {
    ZACT(init);
    ZACT(shutdown);
    ZACT(toneRise);
    ZACT(takeOnMe);
    ZACT(takeOnMePush);
    ZPROP(_output_enable);
    ZPROP(_duty);
    ZPROP(_max_duty);
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


// TODO, globals, ugh

// will this avoid valgrind memmleak?
BeepPwm& gBeepPwm=BeepPwm::getInstance();



int syswr(ctext filename,int i) {
#ifdef NOGPIO
    printf("syswr: %s,%d\n",filename,i);

    return 0;
#endif
    z_string s=i;
    FILE* fd = fopen(filename, "wb");
    if (!fd) {
        perror("pwm write error");
        return Z_ERROR_MSG(zs_io_error,"PWM Error writing %d to %s\n",i,filename);

    }
    fwrite(s.c_str(), s.size(),1,fd); // Export PWM channel 0
    //ZLOG("Writing %s:%s\n",filename,s.c_str());
    fclose(fd);
    return 0;
}
int BeepPwm::_setPwmFreq(int freq,int duty_percent) {


    if (duty_percent>_max_duty)
        duty_percent=_max_duty;

    if (!_output_enable) {
        freq=0;
        duty_percent=0;
    }
#ifdef NOGPIO
    printf("pwm: %d,%d\n",freq,duty_percent);

    return 0;
#endif



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


int BeepPwm::timer_callback(void *)
{
    Note beep;
    if(!_queue.pop(beep)) {
        _setPwmFreq(0,0);
        return 0;

    }

    auto  [freq, delay, duty] = beep;

    if(delay>2000)
    {
        Z_WARN_MSG(zs_bad_parameter,"Buzzer Delay Too Long");
        delay=100;
    }
    if(!delay)
        delay=1;
    if (_enabled) {
            _setPwmFreq(freq,duty);
    }
    else {
        ZDBG("buzzer not enabled\n");

    }

    return delay;
}
void BeepPwm::pushTones(std::initializer_list<Tone> const beeps)
{
    z_status status=init();  if (status) return;

    if(_queue.get_count()>_max_beep_queue) {
        ZDBG("droping Tone, queue full\n");
        return;
    }
    for(auto i : beeps)
    {
        Note n={i.freq,i.duration,_duty};
        _queue.push(n);
    }
    _timer->start_ms_if_not_running(1);
}

void BeepPwm::pushNotes(std::initializer_list<Note> const notes,bool no_limit)
{
    z_status status=init();  if (status) return;

    if (!no_limit) {
        if(_queue.get_count()>_max_beep_queue) {
            ZDBG("droping notes, queue full\n");
            return;
        }
    }

    for(auto i : notes)
    {
        _queue.push(i);
    }
    _timer->start_ms_if_not_running(1);
}

z_status BeepPwm::pushRemoteBeep(RemoteBeep_t *r) {
    z_status status=init();  if (status) return status;
    if (r->count>RemoteBeepMaxLength)
        return zs_bad_parameter;
    if(_queue.get_count()>_max_beep_queue) {
        ZDBG("dropping beeps, queue full\n");

        return zs_device_busy;

    }
    for(int i=0;i<r->count;i++)
    {
        auto note=r->notes[i];
        if (note.duration) {
            //ZDBG("pushing %d,%d,%d\n",note.freq,note.duration,note.duty);
            _queue.push(note);

        }
    }
    // Add delay so beeps do not run together
    _queue.push({0,40,0});
    _timer->start_ms_if_not_running(1);
    return zs_ok;

}


z_status BeepPwm::toneRise()
{
    z_status status=init();  if (status) return status;

    pushTones({
        {1000,50},{0,1000},
        {800,20},{0,50},
        {800,20},{0,50},
        {600,400},{0,200} });
    return zs_ok;
}

z_status BeepPwm::buzz(int f0,int d0,int f1,int d1,int f2,int d2) {

    z_status status=init();  if (status) return status;

    pushTones({{f0,d0},{f1,d1},{f2,d2}});
    return zs_ok;
}

z_status BeepPwm::beep(int tone, int duration, int volume) {
    z_status status=init();  if (status) return status;

    pushNotes({{tone,duration,volume}});
    return zs_ok;

}

z_status BeepPwm::init() {
    if (!_enabled)
        return zs_not_open;
    if (_initialized)
        return zs_ok;




    if (syswr(PWM_CHIP,0)) {
        _exists=false;
        Z_ERROR_MSG(zs_io_error,"PWM chip export failed, will try again later");
        return zs_io_error;
    }
    z_sleep_ms(10);
    if (syswr(PWM_PATH "enable",0)){
        _exists=false;
        Z_ERROR_MSG(zs_io_error,"PWM enable failed, will try again later");
        return zs_io_error;
    }
    if (  _setPwmFreq(0,0)) {
        _exists=false;
        Z_ERROR_MSG(zs_io_error,"PWM does not exists, disabling");
        return zs_io_error;


    }
    _initialized=true;
    if(!_timer)
        _timer=CREATE_TIMER(BeepPwm::timer_callback );
    return zs_ok;
}
z_status BeepPwm::shutdown() {
#ifndef NOGPIO
    if (!_enabled)
        return zs_not_open;
    _setPwmFreq(0,0);
#endif

    return zs_ok;
}
