//
// Created by ac on 11/12/20.
//
#include "app0.h"
#include "root.h"
#include <filesystem>
//#include <openssl/ossl_typ.h>

ZMETA_DEF(App0);

z_time ts_start;
#define DBGL(...) { z_time now; now.set_now();get_debug_logger().time_mark(now-ts_start);get_debug_logger().format_append(__VA_ARGS__); ZDBGS<<'\n';   }

#undef DBGL
#define DBGL(...)

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
    if(!_open)
        return zs_ok;
    _timer->stop();
    root.getReader().stop();



    _reading=false;
    _file_raw.close_copy();
    _file_filtered.close_copy();
    std::unique_lock<std::mutex> mlock(_mutex_tags);

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

z_status App0::start()
{
    ctext msg="error";
    open();
    if(_reading)
        return zs_already_open;


    _t_started.set_now();
    z_status s=zs_ok;
    if (_record_raw) {
        s=_file_raw.open_new(_file_path_record,"raw",_t_started);
    }

    if (_record_filtered) {
        s=_file_filtered.open_new(_file_path_record,"filter",_t_started);
    }

    if (s==zs_ok)
    {
        s= root.getReader().start();
        if(s==zs_ok)
        {
            _reading=true;
            _recording=true;
            _timer->start(_default_timer_period, true);
            msg="reading started";
            _t_started=root.getReader().getTimeReadingStart();
        }
    }
    ts_start=_t_started;
    // update server with status
    if (s)
    {
        return Z_ERROR_MSG(s, msg);
    }
    return s;
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
    js.key_bool("recording",root.app.is_recording());
    js.keyval_int("session",root.getReader()._session);
    js.keyval("status", zs_get_status_text((z_status)status));
    js.key_bool("error",status!=zs_ok );

    js.keyval("msg",msg);

    js.obj_end();
    return js.as_string();

}

bool App0::callbackQueueEmpty()
{
    _file_raw.flush();
    _file_filtered.flush();
    root.web_server.complete_all();

    return true;
}
z_time RfidTag::processRead(RfidRead *r, RfidReadConsumer& rc) {

    _ts_last_time_seen=r->_time_stamp;
    if (!_ts_first_time_seen)
        _ts_first_time_seen=_ts_last_time_seen;
    _ant_mask|=r->_antNum;
    _count++;
     z_time ts=r->_time_stamp;


    if (_rssi_high < r->_rssi) {
        _rssi_high = r->_rssi;
        _ts_rssi_high =r->_time_stamp;
        if (_state==fr_type_arrived) {
            _ts_next_check_required=ts + (U64)rc._minimum_log_time_ms;


        }
        if (_state==fr_type_new) {
            _ts_next_check_required=ts + (U64)rc._peak_window_ms;


        }

    }
    else {
        if (_state>fr_type_new) {
            _ts_next_check_required=ts + (U64)rc._presence_window_s*1000;
        }
    }
    DBGL("READ %s at %llu  check in is %llu\n",_epc.c_str(),ts.get_t(),_ts_next_check_required.get_t());

    return _ts_next_check_required;

}
bool RfidTag::processCheck( RfidReadConsumer& rc,z_time now) {

    U64 diff=now - _ts_last_time_seen;
    DBGL("CHECK %s  last seen %llu ms\n",_epc.c_str(),diff);

    if (diff>= rc._presence_window_s*1000) {
        // write out exit

        if ( // if has been more than X ms since last time logged
            (_ts_last_time_seen-_ts_time_logged > rc._minimum_log_time_ms) ||
            (// if tag has been present more than X seconds
             (_ts_last_time_seen-_ts_first_time_seen) > rc._start_detection_s*1000
            ))
            _state=fr_type_departed;
        else
            _state=fr_type_delete;


        return true;
    }
    if (_state==fr_type_new) {
        if (now-_ts_rssi_high >=rc._peak_window_ms) {
            _state=fr_type_arrived;
            return true;

        }
        return false;
    }

    if (_state>=fr_type_arrived) {
        if (_rssi_high_logged<_rssi_high) {
            if (now - _ts_time_logged > rc._minimum_log_time_ms) {
                _state=fr_type_higher_rssi;

                return true;
            }
        }
    }
    _ts_next_check_required=now+(rc._presence_window_s*1000- diff);
    DBGL("SET NEXT CHECK %s   to %llu \n",_epc.c_str(),_ts_next_check_required.get_t());

    return false;
}
void RfidTag::writeOut(z_stream& s,z_time base,bool clear) {

    z_time ts;
    if (_state==fr_type_departed)
        ts=_ts_last_time_seen;
    else
        ts=_ts_rssi_high;



    if (0) {
        U64 elap=ts-base;
        s.format_append("%6u.%03u,"  ,elap/1000,elap%1000);

    }
    s<<_index;
    s,ts.to_string_ms(true);




    s, ts.get_t() , _ant_mask , _count, _rssi_high, _epc;

    switch (_state) {
        case fr_type_arrived:
            s,"ARR";
            break;
        case fr_type_departed:
            s,"DEP";

            break;
        default:
            s,"HI";

            break;
    }

    s<<'\n';
    _count=0;
    _rssi_high_logged=_rssi_high;
    _ant_mask=0;
    _ts_time_logged=_ts_rssi_high;
    s.flush();

}
void App0::beep() {
    if (_beep)
        root.gpio.beeper.beep(50);
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
    DBGL("timer callback ");

    auto it = _tags.start();
    int next_callback=_default_timer_period;
    while (it!=_tags.end()) {
        RfidTag* t=it->second;
        z_string epc=it->first;

        if (t->processCheck(*this,now)) {
            if (t->_state>fr_type_delete) {
                t->_index=_index++;
                //if (_debug_reads)                     t->writeOut( ZDBGS,_t_started,true);
                if (_record_filtered)
                    t->writeOut( _file_filtered.get_stream(),_t_started,true);

                beep();


            }

            if (t->isDeparted()) {
                //Z_ERROR_LOG("deleting: %s ",t->_epc.c_str());
                it=_tags.erase(it);

                continue;

            }
            t->_ts_next_check_required=t->_ts_rssi_high + (U64) _presence_window_s*1000;
            DBGL("set %s check in to %llu",t->_epc.c_str(),t->_ts_next_check_required.get_t());

        }
        U64 next=next_callback;
        if (now > t->_ts_next_check_required) {
            Z_ERROR_LOG("Next check required is past for: %s, now=%llu ts=%llu\n",t->_epc.c_str(),now.get_t(),t->_ts_next_check_required.get_t());
            next=1000;
        }
        else {
            next=t->_ts_next_check_required-now;

        }
        if (next<next_callback)
            next_callback=next;

        ++it;
    }
    if (next_callback<=0) {
        Z_ERROR_LOG("callback time ? %d ",next_callback);
        next_callback=1;
    }
    return next_callback;
}

bool App0::callbackRead(RfidRead* read)
{
    std::unique_lock<std::mutex> mlock(_mutex_tags);
    z_time now=z_time::get_now_ms();

    try {
        z_string epc;
        read->getEpcString(epc);


        if (_record_raw) {
            _file_raw.writeRfidRead(read, epc);
        }
        if(_debug_reads)
        {
           // ZDBGS << read->get_ms_epoch() << ":" << read->_antNum << ":" << read->_rssi << ":" << epc <<"\n";
        }
        RfidTag *pTag = _tags.getobj(epc);
        if (!pTag) {
            pTag = z_new RfidTag();
            pTag->_epc=epc;
            _tags.add(epc, pTag);
        }

        U64 needs_check_in= pTag->processRead(read,*this)-now;


        _timer->set_minimum_ms_left(needs_check_in);
        DBGL("needs_check_in=%d",needs_check_in);

    }
    catch(...)
    {
        Z_THROW_MSG(zs_internal_error,"Exception writing to read log file");
    }
    return true;
}

