//
// Created by ac on 11/12/20.
//
#include "app0.h"
#include "root.h"
#include <filesystem>
//#include <openssl/ossl_typ.h>

ZMETA_DEF(App0);


App0::App0()
{


}

z_status App0::remote_quit()
{
    // THIS IS CALLED FROM WS SERVER CONTEXT - CANNOT QUIT FROM HERE
    printf("QUIT REQUEST\n");

    root.quit_notify();
    return zs_ok;
}
z_status App0::shutdown()
{
    printf("App0 quiting\n");
    stop();
    //
    if(_timer)
        root.timerService.remove_timer(_timer);
    return zs_ok;
}
z_status App0::initialize()
{
    return zs_write_error;
}

z_status App0::open()
{
    if(_open)
        return zs_ok;

    _record_file_fullname=_file_path_record+"/"+_record_file_name;

    std::error_code ec;
    std::filesystem::create_directory(_file_path_record.c_str(),ec);
    if (ec.value()) {
        Z_ERROR_MSG(zs_bad_parameter,"Could not create directory record file!");
    }
    root.getReader().register_consumer(this);

    if(!_timer)
        _timer=root.timerService.create_timer_t(this,&App0::timer_callback,0    );
    //root.gpio.ledRed.on();

    _open=true;

    return zs_ok;
}
z_status App0::close()
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
z_status App0::run()
{

    return open();
}
z_status App0::stop()
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
            _record_file.close();
            _close_copy_file();

        }



    }
    _tags.delete_all();
    //root.gpio.ledRed.on();
    //root.gpio.ledGreen.off();

    return zs_ok;
}

z_status App0::setup_reader_live(z_json_obj &settings)
{
    if(root.getReader().isReading())
        return zs_access_denied;
    //_write_to_file=true;
    settings.print(stdout_json);
    int power=settings.get_int("powerLevel");
    int filterTime=settings.get_int("filterTime");
    int session=settings.get_int("session");
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
z_status App0::_start_new_file()
{
    bool restart=_reading;
    stop();
    z_string time_str;
    z_time time_now = z_time::get_now();
    time_now.string_format(time_str, "-%Y_%m_%d_%H_%M_%S",true);
    if (!_file_path_record) {
        return Z_ERROR_MSG(zs_bad_parameter,"Record path not set");

    }
    _record_file_name = "live-"+  std::to_string(time_now.get_t()) + time_str + ".txt";
    _record_file_fullname=_file_path_record+"/"+_record_file_name;
    //_write_to_file=true;
    root.console.savecfg();

    return zs_ok;
}

z_status App0::start()
{
    ctext msg="error";
    open();
    if(_reading)
        return zs_already_open;


    _t_started.set_now();
    z_status s=zs_ok;
    if (_write_to_file) {
        s=_start_new_file();
        if (s)
            return s;
        //z_string fullname =  _file_path_record+_record_file_name;


        s = _record_file.open(_record_file_fullname, "ab");
        if (s)
        {
            msg= "Could not open record file!";
            Z_ERROR_MSG(s,"Could not open record file: \"%s\" : %s ",_record_file_fullname.c_str(),zs_get_status_text(s));

        }
    }



    if (s==zs_ok)
    {
        s= root.getReader().start();
        if(s==zs_ok)
        {
            _reading=true;
            _recording=true;
            _timer->start(_tag_cleanup_timer_ms, true);
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

z_status App0::_close_copy_file()
{
    stop();
    if (!_write_to_file)
        return zs_ok;
    z_string time_str;
    z_time time_now = z_time::get_now();
    _recording=false;
    _t_started.string_format(time_str, "%Y_%m_%d_%H_%M_%S-",true);
    z_string new_name =_file_path_record+"/reads-"+time_str +  std::to_string(_t_started.get_t()) +"-" + std::to_string(time_now.get_t())  + "-.txt";
    try {
        // Copy the file
        std::error_code ec;
        std::filesystem::rename(_record_file_fullname.c_str(), new_name.c_str(),ec);
        if (ec.value()) {
            ZT("Error moving file %s",ec.message().c_str());
        }

    } catch (const std::filesystem::filesystem_error& e) {
    }
    root.console.savecfg();

    return zs_ok;
}

int App0::add_json_status(z_json_stream &js) {
    js.keyval("reads_filename",_record_file_name);
    js.keyval("reads_path",_file_path_record);
    js.key_bool("reading",is_reading());
    root.getReader().json_status_get(js);
    return 0;

}

z_string App0::createJsonStatus(int status, ctext msg,bool ack)
{
    z_json_str_stream js;
    js.obj_start();
    js.keyval("command",
              (ack? "reader_ack" : "status_reader"));
    js.keyval("reads_filename",_record_file_name);
    js.keyval("reads_path",_file_path_record);
    js.key_bool("beeps",_beep);
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

bool App0::callbackQueueEmpty()
{
    if(_record_file.is_open())
        _record_file.flush();
    root.web_server.complete_all();

    return true;
}
int  App0::timer_callback(void*)
{
    z_time now;
    now.set_now();
    ;
    std::unique_lock<std::mutex> mlock(_mutex_tags);
    /*
    ZDBG("tags:");
    for (auto const& [key, val] : _tags)
    {
            ZDBGS << key <<',';
    }
    ZDBG("\n");
    */
    auto it = _tags.start();
    int next_callback=_tag_cleanup_timer_ms;
    while (it!=_tags.end()) {
        RfidTag* tag=it->second;
        z_string epc=it->first;
        if (now - tag->_ts_last_time_seen > _min_split_time_s*1000) {

            ZDBGS <<"DONE: "<<  epc << " - "<< tag->_rssi_high << " - " <<  tag->_ts_rssi_high.to_string_ms()<<"\n";

            //ZDBG("erasing old tag: %s\n",epc.c_str());
            it=_tags.erase(it);
            continue;
        }
        U64 rssi_age=(now - tag->_ts_rssi_high).total_milliseconds() ;
        if ( rssi_age>= _peak_window_ms) {
            if (!tag->_counted) {
                ZDBGS <<"COUNTING: "<<  epc << " - "<< tag->_rssi_high << " - " <<  tag->_ts_rssi_high.to_string_ms()<<"\n";
                tag->_counted=true;
                //printf("\a");
                root.gpio.beeper.beep(100);
            }
        }
        else {
            if (rssi_age && (next_callback>rssi_age))
                next_callback=rssi_age;
        }


        it++;
    }
    return next_callback;
}

bool App0::callbackRead(RfidRead* read)
{
    std::unique_lock<std::mutex> mlock(_mutex_tags);
    U64 now=z_time::get_now_ms();

    try {
        z_string epc;
        read->getEpcString(epc);
        if (_single_tag_test) {
            if (_test_tag != epc)
                return true;
        }
        if(_print_reads)
        {
           // ZDBGS << read->get_ms_epoch() << ":" << read->_antNum << ":" << read->_rssi << ":" << epc <<"\n";
        }
        RfidTag *pTag = _tags.getobj(epc);
        if (!pTag) {
            pTag = z_new RfidTag();
            //ZDBG("adding new tag: %s\n",epc.c_str());

            _tags.add(epc, pTag);
        }
        pTag->_ts_last_time_seen=read->_time_stamp;
        if (pTag->_rssi_high < read->_rssi) {
            z_time ts=read->_time_stamp;
            //ts.set_ptime_ms();
            ZDBGS << epc << " - "<< read->_rssi <<" - " <<  ts.to_string_ms()<<" - " << pTag->_counted<<"\n";
            if (pTag->_counted) {
                root.gpio.beeper.beep(50);

            }
            else {
                if (_timer->get_ms_left()> _peak_window_ms)
                    _timer->set_ms_left(_peak_window_ms);
            }
            pTag->_rssi_high = read->_rssi;
            pTag->_ts_rssi_high =read->_time_stamp;
            return true;
        }

    }
    catch(...)
    {
        Z_THROW_MSG(zs_internal_error,"Exception writing to read log file");
    }
    return true;
}

