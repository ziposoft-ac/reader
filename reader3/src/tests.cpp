//
// Created by ac on 11/13/20.
//

#include "tests.h"
#include "root.h"
#include "zipolib/z_error.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/eventfd.h>
ZMETA(Tests)
{

    ZOBJ(heatTest);
    ZOBJ(flashleds);
    ZOBJ(gpioOnOff);
    //ZOBJ(wsBlast);
    ZOBJ(timer);
    ZOBJ(thread);
    //ZOBJ(thread2);
    ZOBJ(pipe);
    ZACT(shutdown);

};

ZMETA(TestHeatTest) {
    ZBASE(TestTimer);

    ZPROP(_max_temp_shutoff);
    ZPROP(_read_pause_time);
}

ZMETA(TestPipe)
{
   // ZACT(openpipe);
    ZCMD(write, ZFF_CMD_DEF, "write",
         ZPRM(z_string, data, "1234", "data to wrtie", ZFF_PARAM)
    );
    ZACT(read);

};

z_status Tests::shutdown()
{
    flashleds.stop();
    //wsBlast.stop();
    gpioOnOff.stop();
    timer.stop();

    return zs_ok;
}
ZMETA(TestTimers)
{
    ZBASE(TestTimer);
};
ZMETA(TestGpioOnOff)
{
    ZBASE(TestTimer);
    ZPROP(_gpioNum);
};
ZMETA(TestLedFlash)
{

    ZBASE(TestGpioOnOff);

};
ZMETA_DEF(TestThread);

z_status TestPipe::openwrite()
{
    if(!_fPipe)
    {
        mkfifo(_name,0666);
        _fPipe = open(_name,(  O_WRONLY));
    }
    return zs_ok;
}
z_status TestPipe::openread()
{
    if(!_fPipe)
    {
        mkfifo(_name,0666);
        _fPipe = open(_name,(O_RDWR));
    }
    return zs_ok;
}
z_status TestPipe::read()
{
    openread();
    _fQuitEvent=eventfd(0,EFD_CLOEXEC);

    pollfd fds[]={{_fPipe,POLLIN,0},{_fQuitEvent,POLLIN,0}};
    bool running=true;
    char buf[81];
    size_t n=0;
    do {
        int event=::poll(fds, 2, 1000);
        if(event==1)
        {
            n= ::read(_fPipe,buf,80);
            buf[n]=0;
            zout<< buf <<"\n";

        }
        if(event==0)
        {
        }
        zout.flush();
        if(root.shuttingDown())
            break;
    }while(running);
    return zs_ok;


}
z_status TestThread::start()
{
    if (_running)
        return zs_already_open;
    _quit = false;

    _thread_handle = std::thread(&TestThread::_thread, this);
    _running = true;

    return zs_ok;

}
z_status TestThread::stop()
{

    _quit = true;
    if (_thread_handle.joinable())
        _thread_handle.join();
    _running = false;


    return zs_ok;

}
z_status TestTimer::stop()
{
    if(_timer)
        _timer->stop();
    onStop();
    _running=false;

    return zs_ok;
}
z_status TestTimer::start()
{
    _count=0;
    _state=0;
    z_status status=onStart();
    if(status!=zs_ok) {
        Z_ERROR_MSG(status,"start timer failed");
        return status;

    }

    bool res=onCallback(0);
    if(!res)
        return zs_ok;

    if(!_timer)
        _timer=root.timerService.create_timer_t(this,&TestTimer::timer_callback,0    );
    _timer->start(_time_off);
    return zs_ok;
}

int TestTimer::timer_callback(void* context)
{
    int ms=onCallback(context);
    if(!ms)
    {
        _running=false;
        return 0;
    }
    return ms;
}
z_status TestHeatTest::onStart() {
    if (root.cfmu804.stop())
        return zs_io_error;
    auto conf=rfid_config_heattest;
    conf.pauseTime=_read_pause_time;

    if (root.cfmu804.configure(conf))
        return zs_io_error;
    root.cfmu804.config_dump();
    ZLOG("STARTING HEAT TEST: intvl=%d maxtemp=%d pause=%d\n",_time_off,_max_temp_shutoff,_read_pause_time);

    root.cfmu804.start();

    return zs_ok;

}
z_status TestHeatTest::onStop() {
    root.cfmu804.stop();

    ZLOG("STOPPING HEAT TEST: intvl=%d maxtemp=%d pause=%d\n",_time_off,_max_temp_shutoff,_read_pause_time);
    ZLOG("TIME STOP:",z_time::getTimeStrLocal().c_str());


    return zs_ok;

}

int TestHeatTest::onCallback(void* context)
{
    root.cfmu804.stop();
    if(root.shuttingDown())
        return 0;

    int count=root.cfmu804.getReadIndex();
    int temp=root.cfmu804.get_temperature_cmd();
    z_string ts=z_time::getTimeStrLocal();
    double tempf=temp;
    tempf=tempf*9/5+32;
    ZLOG("%s :%d°c %.1lf°f %d\n",ts.c_str(),temp,tempf,count);
    get_default_logger().flush();
    if(temp>_max_temp_shutoff) {
        ZLOG("TEMP EXCESS! STOPPING TEST");
        return 0;

    }
    if(root.shuttingDown())
        return 0;
    root.cfmu804.start();
    return _time_off;
}




z_status TestGpioOnOff::onStop()
{
    root.gpio.set(_gpioNum,false);
    return zs_ok;
}


int  TestGpioOnOff::onCallback(void*)
{
    if(_count++>_iterations)
        return 0;
    _state=!_state;
    root.gpio.set(_gpioNum,_state);
    if(_state)
        return _time_on;
    else
        _iterations--;
    return _time_off;
}

#if 0
z_status TestWsBlast::onStart()
{
    zout <<  "TestWsBlast\n";

    root.server.start();
    return zs_ok;
}

int  TestWsBlast::onCallback(void*)
{
   // zout <<  "TestWsBlast::onCallback\n";

    if(!root.server.isRunning())
        return 0;
    if(_count++>=_iterations)
        return 0;
    z_string msg=std::to_string(_count);
    root.server.sendStr(msg);
    if(_count%3000 == 0)
        zout << msg << "\n";

    return _time_off;
}
#endif
int  TestTimers::onCallback(void*)
{
    // zout <<  "TestWsBlast::onCallback\n";
    _iterations++;
    _count++;
    if(_count*_time_off>5000)
    {
        _count=0;
        zout << "timertest:"<< _iterations<< "\n";

    }

    return _time_off;
}
