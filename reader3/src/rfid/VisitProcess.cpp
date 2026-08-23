//
// Created by ac on 11/12/20.
//
#include "VisitProcess.h"
#include "rfidService/RfidService.h"

#include <filesystem>


ZMETA_DEF(VisitProcess);


ctext default_record_path = "/zs/timer_data/reads";
ctext default_record_path_raw = "/zs/timer_data/raw";


#ifdef DEBUG
static z_time DBG_TS_START;

double get_dbg_time(I64 diff) {
    z_time now;
    now.set_now();
    double t = now - DBG_TS_START + diff;
    return t / 1000;
}

double get_dbg_ts(const z_time &ts) {
    double diff = ts - DBG_TS_START;
    return diff / 1000;
}

static double get_dbg_ts_ms(U64 ts) {
    double diff = ts - DBG_TS_START.in_ms();
    return diff / 1000;
}



#define DBGL(...) { z_time now; now.set_now();get_debug_logger().time_mark(now-DBG_TS_START);get_debug_logger().format_append(__VA_ARGS__); ZDBGS<<'\n';   }
#else
#define DBGL(...) {}
#endif

#undef DBGL
#define DBGL(...)

VisitProcess::VisitProcess() {
}


z_status VisitProcess::shutdown() {
    if (!_open)
        return zs_ok;
    stop();


    _open = false;
    return zs_ok;
}

z_status VisitProcess::initialize() {
    if (_open)
        return zs_ok;

    //gGpio.initialize();


    getRfidReader().register_cb_read(this, [this](RfidRead *r) { return callbackRead(r); });
    getRfidReader().register_cb_queue_empty(this, [this]() {
        _file_raw.flush();
        return true;
    });
    //root.web_server.start();

    if (!_timer_tag_process)
        _timer_tag_process = CREATE_TIMER(VisitProcess::callback_tag_process);
    if (!_timer_write_notify)
        _timer_write_notify = CREATE_TIMER(VisitProcess::callback_write_notify);

    _open = true;
    //root.beeper.pushBeeps( {{1000,50},{1200,50},{1400,50},{0,80}  });
    _last_notify_timestamp=z_time_get_ticks_ms();

    return zs_ok;
}

z_status VisitProcess::run() {
    return initialize();
}

z_status VisitProcess::stop() {
    if (!_open)
        return zs_ok;
    _timer_tag_process->stop();
    _timer_write_notify->stop();
    getRfidReader().stop();

    write_out_all();
    _running = false;

    _reading = false;
    _file_raw.close_copy();
    _file_visits.close_copy();
    std::unique_lock mlock(_mutex_tags);
    printf("deleting tags\n");

    _tags.delete_all();
    //root.beeper.pushBeeps(            {{1500,30},{1000,30},{750,30},{500,100}});
    //gGpio.ledRed.on();
    //gGpio.ledGreen.off();

    return zs_ok;
}


// TODO - unused
z_status VisitProcess::setup_reader_live(z_json_obj &settings) {
    if (getRfidReader().isReading())
        return zs_access_denied;
    //_write_to_file=true;
    settings.print(stdout_json);
    int power = settings.get_int("powerLevel");
    int filterTime = settings.get_int("filterTime");
    int session = settings.get_int("session");
    if ((session < 0) || (session > 3))
        session = 0;
    if ((filterTime < 1) || (filterTime > 1000))
        filterTime = 5;
    if ((power < 10) || (power > 33))
        power = 33;
    return getRfidReader().configure({
        5, 1, 0xf, 0, 3, 30, 0, 0
    });
}

z_status VisitProcess::start_json(z_json_obj &o) {
    _record_raw = o.get_bool("record_raw", _record_raw);
    _beep = o.get_bool("enable_beep", _beep);
    _file_path_record = default_record_path;
    o.get_str("path", _file_path_record);

    return start();
}

z_status VisitProcess::start() {
    ctext msg = "error";
    z_status status = initialize();
    if (status != zs_ok)
        return status;
    if (_reading)
        return zs_already_open;


    _t_started.set_now();
    z_status s = zs_ok;
    if (_record_raw) {
        s = _file_raw.open_new(_file_path_record_raw, "raw", _t_started);
    }

    if (_record_visits) {
        s = _file_visits.open_new(_file_path_record, "visit", _t_started);
    }

    if (s == zs_ok) {
        s = getRfidReader().start();
        if (s == zs_ok) {
            _reading = true;
            _running = true;
            msg = "reading started";
            _t_started = getRfidReader().getTimeReadingStart();
        }
    }
    DBG_TS_START = _t_started;
    // update server with status
    if (s) {
        stop();
        return Z_ERROR_MSG(s, msg);
    }
    //root.beeper.pushBeeps(         {{500,30},{0,30},{750,30}});
    return s;
}

bool VisitProcess::is_reading() {
    return getRfidReader().isReading();
}

int VisitProcess::get_live_tag_visits(z_json_stream &js) {
    //ZDBG("get_live_tag_visits wait...\n");
    std::unique_lock mlock(_mutex_tags);
    //ZDBG("get_live_tag_visits done\n");

    js.obj_val_start("live_visits");

    for (auto const& [key, t] : _tags) {

        t->writeJson(js);
    }
    js.obj_end();



    return zs_ok;
}

int VisitProcess::add_json_status(z_json_stream &js) {
    js.set_pretty_print(true);
    get_live_tag_visits(js);
    js.obj_val_start("visitProc");

    js.keyval("file", _file_visits.getLiveFileName());
    js.keyval("reads_path", _file_path_record);
    js.key_bool("reading", is_reading());
    js.key_bool("recording", is_recording());
    js.keyval_int("ts_last_file_write", getLastWriteTimestamp());
    js.keyval_int("ts_last_notify", getLastNotifyTimestamp());
    js.keyval_int("ts_started", (I64) _t_started.in_ms());
    js.obj_end();

    return 0;
}


void VisitProcess::signalWaitingRequests() {

    if (_timer_write_notify->is_running())
        return;
    U64 now=z_time_get_ticks_ms();
    I64 diff = (I64) _last_notify_timestamp-now+_min_notify_ms;

    if (diff <  1) {
        diff=1;
    }
    _timer_write_notify->start_ms_reset( diff);

}
int VisitProcess::callback_write_notify(void *) {
    _last_notify_timestamp = z_time_get_ticks_ms();
    gCallbackVisitNotify();
    return 0;
}


void VisitProcess::beep() {
    //if (_beep)         gGpio.beeper.beep(50);
}


int VisitProcess::callback_tag_process(void *) {
    U64 now=z_time_get_ticks_ms();
    U64 next_callback = 0;
    //DBGL("timer callback ");

    bool signal_write = false;


    // LOCK TAGS
    {
        //ZDBG("callback_tag_process wait...\n");
        std::unique_lock mlock(_mutex_tags);
        //ZDBG("callback_tag_process done\n");
        auto it = _tags.start();
        while (it != _tags.end()) {
            RfidTag *t = it->second;
            z_string epc = it->first;

            if (t->processCheck(*this, now)) {
                if (t->_state == fr_type_peaked) {
                    ZDBG("peaked:%s\n", epc.c_str());
                        signalWaitingRequests();
                    beep();

                }


                if (t->isDeparted()) {
                    if (_record_visits) {

                        t->writeOut(_file_visits.get_stream(),getNewWriteTimestamp());
                        signal_write = true;
                    }
                    //Z_ERROR_LOG("deleting: %s ",t->_epc.c_str());
                    delete t;
                    it = _tags.erase(it);


                    continue;
                }
                DBGL("set %s check in to %6.3lf", t->_epc.c_str(),
                     get_dbg_ts( t->_ts_next_check_required));
            }
            I64 next= t->_ts_next_check_required.in_ms() - now;

            if (next<=0) {
                Z_ERROR_LOG("Next checkin required already expired? for: %s, late=%lld\n", t->_epc.c_str(), next);
                next = 1;
            }
            if ((next_callback==0)||(next < next_callback))
                next_callback = next;

            ++it;
        }
        if (next_callback < 0) {
            Z_ERROR_LOG("callback time ? %d ", next_callback);
            next_callback = 1;
        }
    }

    if (signal_write) {
        getNewWriteTimestamp();
        _file_visits.flush();
        signalWaitingRequests();
    }


    return (int) next_callback;
}


z_status VisitProcess::write_out_all() {
    std::unique_lock mlock(_mutex_tags);
    if (_record_visits) {
        auto it = _tags.start();
        while (it != _tags.end()) {
            RfidTag *t = it->second;
            z_string epc = it->first;


            t->writeOut(_file_visits.get_stream(),getNewWriteTimestamp());
            delete t;
            it = _tags.erase(it);
        }
    }


    return zs_ok;
}


bool VisitProcess::callbackRead(RfidRead *read) {
    //ZDBG("callbackRead\n");

    if (!_running)
        return false;
    z_time now = z_time::get_now_ms();
    bool signal=false;
    try {
        z_string epc;
        read->getEpcString(epc);

        if (_record_raw) {
            _file_raw.writeRfidRead(read, epc);
        }

        {
            std::unique_lock mlock(_mutex_tags);
           // ZDBG("callbackRead locked\n");

            RfidTag *pTag = _tags.getobj(epc);
            if (!pTag) {
                pTag = z_new RfidTag(read, epc,_persistent_index++);
                pTag->_epc = epc;
                _tags.add(epc, pTag);
                signal=true;


            }

            _ts_last_read = read->_time_stamp;

            z_time next_check_in = pTag->processRead(read, *this);

            for (auto const& [key, t] : _tags) {
                if (t->_ts_next_check_required < next_check_in)
                    next_check_in = t->_ts_next_check_required;
            }
            _timer_tag_process->start_ts_reset(next_check_in);
            DBGL("new timer check at=%4.3lf", get_dbg_ts_ms(_timer_tag_process->get_timout_ts()));
        }

    } catch (...) {
        Z_THROW_MSG(zs_internal_error, "Exception writing to read log file");
    }
    if (signal) signalWaitingRequests();
    //ZDBG("callbackRead exit\n");

    return true;
}
#if 1
z_time RfidTag::processRead(RfidRead *r, VisitProcess &rc) {
    _last_rssi = r->_rssi;
    _ts_last_time_seen = r->_time_stamp;
    if (!_ts_first_time_seen)
        _ts_first_time_seen = _ts_last_time_seen;
    _ant_mask = _ant_mask | r->_antNum;
    _count_total++;
    z_time ts = r->_time_stamp;
    bool hi = false;
    if (_rssi_high < r->_rssi) {
        // new RSSI high
        _ts_next_check_required = ts + (U64) rc._peak_window_ms;
        _rssi_high = r->_rssi;
        _ant_hi = r->_antNum;
        _count_hi = _count_total;
        _ts_rssi_high = r->_time_stamp;
        _state = fr_type_signal_going_up;
        hi = true;
    } else {
    }


    DBGL("%s read %s at  %4.3lf  check in is  %4.3lf", (hi?"HI":"LOW"), _epc.c_str(),
         get_dbg_ts(ts),
         get_dbg_ts(_ts_next_check_required));


    return _ts_next_check_required;
}

bool RfidTag::processCheck(VisitProcess &rc, const z_time& now) {
    const z_time_duration missing_time = now - _ts_last_time_seen;
    const z_time_duration decline_time = now - _ts_rssi_high;

    DBGL("CHECK %s  last seen %llu ms\n", _epc.c_str(), missing_time.total_milliseconds());

    bool write_it_out = false;
    if (_state == fr_type_arrived) // Initial state on creation
        _state = fr_type_signal_going_up; // start look for peak

    if (missing_time.total_milliseconds() >= rc._presence_window_s * 1000) {
        // write out exit

        _state = fr_type_departed;

        return true;
    }
    if (_state == fr_type_signal_going_up) {
        if ((now - _ts_rssi_high) < rc._peak_window_ms) {
            // waiting to see if signal is going up, don't write it
            // come back when peak window expires
            _ts_next_check_required = _ts_rssi_high + (U64) rc._peak_window_ms;
            return false; // it has peaked, so write it out.
        }
        // if the time since last peak is past the peak window,
        _state = fr_type_peaked;

        write_it_out = true;
    }
    if (_state == fr_type_peaked) {
        // waiting for new peak , check in again at end of presense window.
        _ts_next_check_required = _ts_last_time_seen + (U64) rc._presence_window_s * 1000;
        DBGL("%s PEAKED, check again at %4.3lf", _epc.c_str(),
             get_dbg_ts(_ts_next_check_required)
        );
    } else {
        Z_ERROR_LOG("should never get here");
    }

    return write_it_out;
}

/*
*export class ReadVisit
{
count=0;
tsIn=0;
tsOut=0;
tsPeak=0;
rssi=0;
epc="";
antMask=0;
antHi=0;
}

index,epc,ts,in,out,count,rssi,ant,ant_hi


*/
void RfidTag::writeOut(z_stream &s,U64 writeTs) {
    U64 ts = _ts_rssi_high.get_t();
    I64 in = _ts_first_time_seen.get_t() - ts;
    I64 out = _ts_last_time_seen.get_t() - ts;
    s << _index,writeTs, _epc.c_str(), ts,
            in, out, _count_total, _rssi_high, _ant_mask, _ant_hi;
    s << '\n';
}
/*
export class ReadVisit
{
    index=0;
    epc="";
    ts=0;
    toIn=0;
    toOut=0;
    count=0;
    rssi=0;
    antMask=0;
    antHi=0;
    rid:ReaderID=ReaderID.local;
    // remove this??
    //maxMissing=0;
}

 */
void RfidTag::writeJson(z_json_stream& s) {
    U64 ts = _ts_rssi_high.get_t();
    s.keyval("epc",_epc);
    s.keyval_int("ts",ts);
    s.keyval_int("count",_count_total);
    s.keyval_int("rssi",_rssi_high);
    s.keyval_int("antMask",_ant_mask);
    s.keyval_int("antHi",_ant_hi);
    s.keyval_int("toIn", _ts_first_time_seen.get_t() - ts);
    s.keyval_int("toOut", _ts_last_time_seen.get_t() - ts);


}

#endif
