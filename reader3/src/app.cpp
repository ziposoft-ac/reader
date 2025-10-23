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
    if(_record_file.is_open())
        _record_file.flush();

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
    read_stop();
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

    _record_file_fullname=_file_path_record+"/"+_record_file_name;

    root.getReader().register_consumer(this);

    if(!_timer)
        _timer=root.timerService.create_timer_t(this,&App::timer_callback,0    );
    //root.gpio.ledRed.on();

    _open=true;
    if(_buzzer)
        root.gpio.buzzer.pushBeeps(
                {{1000,50},{1200,50},{1400,50},{0,80}
                 });


    return zs_ok;
}
z_status App::close()
{
    if(!_open)
        return zs_ok;
    root.gpio.buzzer.pushBeeps(
            {{1500,50},{1000,50},{500,50},{0,50},
            });
    read_stop();
    _open=false;
    return zs_ok;
}
z_status App::run()
{

    return open();
}
z_status App::read_stop()
{
    if(_open)
    {
        if(_reading)
        {
            if(_buzzer)
            root.gpio.buzzer.pushBeeps(
                    {{1500,30},{1000,30},{750,30},{500,100}});
        }
        _timer->stop();

        root.getReader().stop();
        _reading=false;

        if (_recording) {
            _recording=false;
            _record_file.close();
            _close_copy_file();

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
    _write_to_file=true;
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
    return root.getReader().setupParams(0xf,power,session,filterTime,5);


}
z_status App::_start_new_file()
{
    bool restart=_reading;
    read_stop();
    z_string time_str;
    z_time time_now = z_time::get_now();
    time_now.string_format(time_str, "-%Y_%m_%d_%H_%M_%S",true);
    _record_file_name = "live-"+  std::to_string(time_now.get_t()) + time_str + ".txt";
    _record_file_fullname=_file_path_record+"/"+_record_file_name;
    _write_to_file=true;
    root.console.savecfg();

    return zs_ok;
}

z_status App::start()
{
    ctext msg="error";
    open();
    if(_reading)
        return zs_already_open;
    if(_buzzer)

    root.gpio.buzzer.pushBeeps(
            {{500,30},{0,30},{750,30}});

    _t_started.set_now();

    _start_new_file();
    //z_string fullname =  _file_path_record+_record_file_name;
    z_status s = _record_file.open(_record_file_fullname, "ab");
    if (s)
    {
        msg= "Could not open record file!";
        Z_ERROR_MSG(s,"Could not open record file: \"%s\"  ",_record_file_fullname.c_str());
    }
    else
    {
        /*
        z_string current=_file_path_record+"current.csv";
        std::filesystem::remove(current.c_str());
        std::filesystem::create_symlink(_record_file_name.c_str(),current.c_str() );
         */
        s= root.getReader().start();
        if(s==zs_ok)
        {
            _reading=true;
            if(_write_to_file)
                _recording=true;
            _timer->start(_file_flush_seconds, true);
            //root.gpio.ledGreen.on();
            //root.gpio.ledRed.off();
            msg="reading started";
        }
    }

    // update server with status
    if (s)
    {
        return Z_ERROR_MSG(s, msg);
    }
    return s;
}

z_status App::_close_copy_file()
{
    read_stop();
    z_string time_str;
    z_time time_now = z_time::get_now();

    _t_started.string_format(time_str, "%Y_%m_%d_%H_%M_%S-",true);
    z_string new_name =_file_path_complete+"/reads-"+time_str +  std::to_string(_t_started.get_t()) +"-" + std::to_string(time_now.get_t())  + "-.txt";
    try {
        // Copy the file
        std::error_code ec;
        bool success=std::filesystem::copy_file(_record_file_fullname.c_str(), new_name.c_str(),ec);
        if (!success) {
            ZT("Error copying file %s",ec.message().c_str());
        }

    } catch (const std::filesystem::filesystem_error& e) {
    }
    root.console.savecfg();

    return zs_ok;
}

int App::add_json_status(z_json_stream &js) {
    js.keyval("reads_filename",_record_file_name);
    js.keyval("reads_path",_file_path_record);
    js.key_bool("reading",is_reading());
    root.getReader().add_json_status(js);
    return 0;

}

z_string App::createJsonStatus(int status, ctext msg,bool ack)
{
    z_json_str_stream js;
    js.obj_start();
    js.keyval("command",
              (ack? "reader_ack" : "status_reader"));
    js.keyval("reads_filename",_record_file_name);
    js.keyval("reads_path",_file_path_record);
    js.key_bool("beeps",_buzzer);
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
    if(_write_to_file)
        _record_file.flush();
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
        if(_record_file.is_open())
            _record_file << timestamp << ','<< read->_antNum << ',' <<  read->_rssi <<',' << epc<< '\n';
        root.web_server.complete_all();
    }
    catch(...)
    {
        Z_THROW_MSG(zs_internal_error,"Exception writing to read log file");
    }
    return true;
}
/* ORIG
bool App::callbackRead(RfidRead* read)
{
    try {
        z_string epc;
        read->getEpcString(epc);
        if(_print_reads)
        {
            zout << read->get_ms_epoch() << ":" << read->_antNum << ":" << read->_rssi << ":" << epc <<"\n";
            zout.flush();
        }
        RfidTag *pTag = _tags.getobj(epc);
        if (!pTag) {
            pTag = z_new RfidTag();
            _tags.add(epc, pTag);
        }
        U64 timestamp = read->get_ms_epoch();
        U64 missing_time_ms = timestamp - pTag->_last_time_seen;
        pTag->_last_time_seen = timestamp;
        if (missing_time_ms > (_min_split_time * 1000)) {
            pTag->_count++;
            read->_recorded=true;
            //root.gpio.ledYellow.flash(1);
            if(_buzzer)
            {
                root.gpio.buzzer.beepDiminishing({2000,100});
            }
            if(_write_to_file)
            {
                if(_record_file.is_open())
                _record_file << timestamp << ','<< read->_antNum << ',' <<  read->_rssi <<',' << epc<< '\n';
            }
        }
        root.web_server.complete_all();
    }
    catch(...)
    {
        Z_THROW_MSG(zs_internal_error,"Exception writing to read log file");
    }
    return true;
}


*/