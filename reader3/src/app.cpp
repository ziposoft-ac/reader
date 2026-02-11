//
// Created by ac on 11/12/20.
//
#include "app.h"
#include "root.h"
#include <filesystem>
//#include <openssl/ossl_typ.h>

ZMETA_DEF(App);


App::App()
{


}
int  App::timer_callback(void*)
{
    if(_file.is_open())
        _file.flush();

    //NOT USED
    return _file_flush_seconds;
}
z_status App::remote_quit()
{
    // THIS IS CALLED FROM WS SERVER CONTEXT - CANNOT QUIT FROM HERE
    printf("QUIT REQUEST\n");

    root.quit_notify();
    return zs_ok;
}
z_status App::shutdown()
{
    printf("App quiting\n");
    stop();
    //
    if(_timer)
        root.timerService.remove_timer(_timer);
    return zs_ok;
}
z_status App::initialize()
{
    return zs_write_error;
}

z_status App::open()
{
    if(_open)
        return zs_ok;

    root.getReader().register_consumer(this);

    if(!_timer)
        _timer=root.timerService.create_timer_t(this,&App::timer_callback,0    );
    //root.gpio.ledRed.on();

    _open=true;
    if(_beepPwm)
        root.gpio.beepPwm.pushBeeps(
                {{1000,50},{1200,50},{1400,50},{0,80}
                 });


    return zs_ok;
}
z_status App::close()
{
    if(!_open)
        return zs_ok;
    root.gpio.beepPwm.pushBeeps(
            {{1500,50},{1000,50},{500,50},{0,50},
            });
    stop();
    _open=false;
    return zs_ok;
}
z_status App::run()
{

    return open();
}
z_status App::stop()
{
    if(_open)
    {
        if(_reading)
        {
        }
        _timer->stop();

        root.getReader().stop();
        _reading=false;

        if (_recording) {
            _recording=false;
            _file.close_copy();

        }



    }
    _tags.delete_all();
    //root.gpio.ledRed.on();
    //root.gpio.ledGreen.off();

    return zs_ok;
}

z_status App::setup_reader_live(z_json_obj &settings)
{
    if(root.getReader().isReading())
        return zs_access_denied;
    //_write_to_file=true;
    _broadcast= true;
    settings.print(stdout_json);
    int power=settings.get_int("powerLevel");
    int filterTime=settings.get_int("filterTime");
    int session=settings.get_int("session");
    this->_min_split_time=settings.get_int("recTime");
    if((session<0)||(session>3))
        session=2;
    if((filterTime<1)||(filterTime>1000))
        filterTime=5;
    if((power<10)||(power>30))
        power=30;
    return root.getReader().configure({
        5,1,0xf,0,3,30,0,0
    });


}

z_status App::start()
{
    open();
    if(_reading)
        return zs_already_open;


    _t_started.set_now();

    z_status s=_file.open_new(_file_path_record,"raw",_t_started);
    if (s)
        return s;
        //z_string fullname =  _file_path_record+_record_file_name;


    s= root.getReader().start();
    if(s==zs_ok)
    {
        _reading=true;
        _recording=true;
        _timer->start(_file_flush_seconds, true);
    }

    // update server with status
    if (s)
    {
        return Z_ERROR_MSG(s,"Could not start reader");
    }
    return s;
}


int App::add_json_status(z_json_stream &js) {
    js.keyval("reads_filename",_file.getLiveFileName());
    js.keyval("reads_path",_file_path_record);
    js.key_bool("reading",is_reading());
    root.getReader().json_status_get(js);
    return 0;

}

z_string App::createJsonStatus(int status, ctext msg,bool ack)
{
    z_json_str_stream js;
    js.obj_start();
    js.keyval("command",
              (ack? "reader_ack" : "status_reader"));
    js.keyval("reads_filename",_file.getLiveFileName());
    js.keyval("reads_path",_file_path_record);
    js.key_bool("beeps",_beepPwm);
    js.key_bool("reading",root.app.is_reading());
    js.key_bool("recording",root.app.is_recording());
    js.keyval_int("power",root.getReader()._power);
    js.keyval_int("ant_config",root.getReader()._antenna_config);
    js.keyval_int("ant_mask",root.getReader()._antenna_mask);
    js.keyval_int("ant_detected",root.getReader()._antenna_detected);
    js.keyval_int("qValue",root.getReader()._qvalue);
    js.keyval_int("session",root.getReader()._session);
    js.keyval("status", zs_get_status_text((z_status)status));
    js.key_bool("error",status!=zs_ok );

    js.keyval("msg",msg);

    js.obj_end();
    return js.as_string();

}

bool App::callbackQueueEmpty()
{
    if(_file.is_open())
        _file.flush();
    return true;
}

bool App::callbackRead(RfidRead* read)
{
    try {
        z_string epc;
        read->getEpcString(epc);
        U64 timestamp = read->get_ms_epoch();
        read->_recorded=true;
        //root.gpio.ledYellow.flash(1);
        _file.writeRfidRead(read,"a");
    }
    catch(...)
    {
        Z_THROW_MSG(zs_internal_error,"Exception writing to read log file");
    }
    return true;
}
