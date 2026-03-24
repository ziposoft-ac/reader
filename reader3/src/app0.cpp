//
// Created by ac on 11/12/20.
//
#include "app0.h"
#include "root.h"
#include <filesystem>

#include "JsonCmd.h"
//#include <openssl/ossl_typ.h>

ZMETA_DEF(App0);

static z_time ts_start;
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
    return zs_ok;
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
    root.gpio.beepPwm.pushBeeps( {{1000,50},{1200,50},{1400,50},{0,80}  });

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


    _recording=false;

    _reading=false;
    _file_raw.close_copy();
    _file_filtered.close_copy();
    std::unique_lock<std::mutex> mlock(_mutex_tags);
    printf("deleting tags\n");

    _tags.delete_all();
    root.gpio.beepPwm.pushBeeps(
            {{1500,30},{1000,30},{750,30},{500,100}});
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
    root.gpio.beepPwm.pushBeeps(
        {{500,30},{0,30},{750,30}});
    return s;
}
bool App0::is_reading() {
    return root.getReader().isReading();
}


int App0::add_json_status(z_json_stream &js) {
    js.set_pretty_print(true);
    js.obj_val_start("app0");

    js.keyval("file",_file_filtered.getLiveFileName());
    js.keyval("reads_path",_file_path_record);
    js.key_bool("reading",is_reading());
    js.key_bool("recording",is_recording());
    js.keyval_int("last_index",getLastWriteTimestamp());
    js.keyval_int("ts_start",(I64)_t_started.get_ptime_ms());
    js.obj_end();

    return 0;

}


void App0::signalWaitingRequests() {
    if (_last_write_timestamp>_last_notify_timestamp) {
        _last_notify_timestamp=_last_write_timestamp;
        root.web_server.complete_req_type(DELAYED_REQUEST_READS_FILTERED);
        ZDBG("\nsignal waiting for filtered reads\n");
        ZDBGS.flush();

    }
}

bool App0::callbackQueueEmpty()
{
    _file_raw.flush();
    _file_filtered.flush();

    return true;
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
            if (_record_filtered)
                t->writeOut( _file_filtered.get_stream(),t->_state);

            if (t->_state==fr_type_peaked)
                beep();



            if (t->isDeparted()) {
                //Z_ERROR_LOG("deleting: %s ",t->_epc.c_str());
                delete t;
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
    _file_filtered.flush();
    signalWaitingRequests();

    return next_callback;
}

bool App0::callbackRead(RfidRead* read)
{
    z_time now=z_time::get_now_ms();

    try {
        z_string epc;
        bool newtag=false;// UGLY
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
            pTag = z_new RfidTag(read,epc);
            pTag->_epc=epc;
            _tags.add(epc, pTag);

            newtag=true;
        }

        U64 needs_check_in= pTag->processRead(read,*this)-now;
        if (newtag)
            pTag->writeOut(_file_filtered.get_stream(),fr_type_arrived);

        _timer->set_minimum_ms_left(needs_check_in);
        DBGL("needs_check_in=%d",needs_check_in);

    }
    catch(...)
    {
        Z_THROW_MSG(zs_internal_error,"Exception writing to read log file");
    }
    return true;
}
#if 1
z_time RfidTag::processRead(RfidRead *r, RfidReadConsumer& rc) {

    _last_rssi=r->_rssi;
    _ts_last_time_seen=r->_time_stamp;
    if (!_ts_first_time_seen)
        _ts_first_time_seen=_ts_last_time_seen;
    _ant_mask|=r->_antNum;
    _count++;
     z_time ts=r->_time_stamp;

    if (_rssi_high < r->_rssi) {
        // new RSSI high
        _ts_next_check_required=ts + (U64)rc._peak_window_ms;
        _rssi_high = r->_rssi;
        _count_hi=_count;
        _ts_rssi_high =r->_time_stamp;
        _state=fr_type_new_peak;


    }

    DBGL("READ %s at %llu  check in is %llu\n",_epc.c_str(),ts.get_t(),_ts_next_check_required.get_t());

    return _ts_next_check_required;

}
bool RfidTag::processCheck( RfidReadConsumer& rc,z_time now) {

    const z_time_duration missing_time=now - _ts_last_time_seen;
    DBGL("CHECK %s  last seen %llu ms\n",_epc.c_str(),diff);
    if (_state==fr_type_arrived)
        _state=fr_type_new_peak;

    if (missing_time.total_milliseconds() >= rc._presence_window_s*1000) {
        // write out exit

        _state=fr_type_departed;

        return true;
    }
    if (_state==fr_type_new_peak) {
        if (now-_ts_rssi_high >=rc._peak_window_ms) {
            _state=fr_type_peaked;
            return true;

        }
        return false;
    }
    if (_state==fr_type_peaked) {
        _ts_next_check_required=_ts_last_time_seen+ (U64) rc._presence_window_s*1000;
        DBGL("SET NEXT CHECK %s   to %llu \n",_epc.c_str(),_ts_next_check_required.get_t());
    }

    return false;
}
void RfidTag::writeOut(z_stream& s,FilteredReadState type) {

    z_time ts;
    if (_state==fr_type_departed)
        ts=_ts_last_time_seen;
    else
        ts=_ts_rssi_high;





    s<<root.app0.getNewWriteTimestamp();
    s,ts.to_string_ms(true);




    s, ts.get_t() , _ant_mask
    , (_state==fr_type_peaked ? _count_hi:_count)
    , (_state==fr_type_departed ? _last_rssi:_rssi_high)
    , _epc.c_str();

    switch (type) {
        case fr_type_arrived:
            root.gpio.beepPwm.pushBeeps( {{1100,30}});
            s,"ARR";
            break;
        case fr_type_departed:
            s,"DEP";
            root.gpio.beepPwm.pushBeeps( {{500,25}});

            break;
        default:
            s,"HI";
            root.gpio.beepPwm.pushBeeps( {{700,40},{1500,40}});

            break;
    }

    s<<'\n';
    _rssi_high_logged=_rssi_high;
    _ts_time_logged=_ts_rssi_high;
    s.flush();

}
#endif