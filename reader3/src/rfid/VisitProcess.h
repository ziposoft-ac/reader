//
// Created by ac on 11/12/20.
//

#ifndef ZIPOSOFT_APP0_H
#define ZIPOSOFT_APP0_H
#include "pch.h"
#include "zipolib/lockfile.h"

#include "../util/timers.h"
#include "rfid.h"

#include "recordFile.h"


extern ctext default_record_path;
extern ctext default_record_path_raw;

class VisitProcess : public RfidReadConsumer{
    friend z_factory_t<VisitProcess>;
    int _write_count=0;
    bool _open=false;
    bool _reading=false;

    int timer_callback(void*);
    Timer* _timer=0;
    std::mutex _mutex_tags;

    z_obj_map<RfidTag> _tags;
    //PROPS
    int _min_rssi = 70;
    int _filter_epc = true;
    const int _default_timer_period = 5000;
    bool _debug_reads=true;
    bool _beep=true;
    bool _buzzer=true;
    bool _record_raw=false;
    bool _record_visits=true;
    bool _recording=false;
    RecordFile _file_raw;
    RecordFile _file_visits;
    bool _record_tod=true;
    U64 _last_write_timestamp=0;
    U64 _last_notify_timestamp=0;
    U64 _ts_last_read=0;

    RfidReader* _reader;

public:
    VisitProcess();
    virtual z_status open();
    virtual z_status close();
    virtual z_status run();
    virtual z_status shutdown();

    virtual z_status stop();
    virtual z_status start();
    z_time _t_started;
    bool _simulate=true;

    z_string _file_path_record = default_record_path;
    z_string _file_path_record_raw =default_record_path_raw;
    U64 getNewWriteTimestamp() {
        U64 ts=z_time::get_now_ms();
        if (ts<=_last_write_timestamp)
            _last_write_timestamp++;
        else
            _last_write_timestamp=ts;
        return _last_write_timestamp;
    }
    U64 getLastWriteTimestamp()  { return _last_write_timestamp; }
    bool is_reading() ;
    bool is_recording() const { return _recording; }

    virtual z_status setup_reader_live(z_json_obj &settings);

    z_status get_live_tag_visits(Visits &visits);
    virtual z_status remote_quit();

    virtual z_status start_json(z_json_obj& o);

    void beep();
    bool callbackRead(RfidRead* r) override;
    bool callbackQueueEmpty() override;
    virtual void signalWaitingRequests();


    int add_json_status(z_json_stream &js);


    z_status simulate_on() ;
    z_status simulate_off();

    RfidReader& getReader() { return *_reader; }

};


ZMETA_DECL(VisitProcess) {

    ZPROP(_file_path_record);
    //ZPROP(_file_path_complete);
    ZPROP(_beep);
    ZPROP(_buzzer);
    ZPROP(_peak_window_ms);
    ZPROP(_presence_window_s);
    ZPROP(_record_tod);
    ZPROP(_record_raw);
    ZPROP(_record_visits);
    //ZPROP(_start_detection_s);
    //ZPROP(_minimum_log_time_ms);
    ZPROP(_min_rssi);
   // ZPROP(_test_tag);
    ZPROP(_filter_epc);
    //ZPROP(_single_tag_test);
    ZPROP(_recording);
    //ZPROP(_broadcast);
    ZPROP(_debug_reads);
    //#define ZACT(_ACT_) ZACT_X(_ACT_,#_ACT_,ZFF_ACT_DEF,"")
    ZACT_X(stop,"stop",ZFF_ACT_DEF,"stop reading");
    ZACT(start);
   // ZACT(start_new_file);
    ZACT(run);
    ZACT(open);
    ZACT(close);
    ZACT(shutdown);
    ZACT(remote_quit);
    ZSTAT(is_reading);
    ZSTAT(getLastWriteTimestamp);

}

#endif //ZIPOSOFT_APP_H