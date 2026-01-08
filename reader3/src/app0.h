//
// Created by ac on 11/12/20.
//

#ifndef ZIPOSOFT_APP0_H
#define ZIPOSOFT_APP0_H
#include "pch.h"

#include "timers.h"
#include "rfid.h"



class App0 : public RfidReadConsumer{
    friend z_factory_t<App0>;
    int _write_count=0;
    bool _open=false;
    bool _reading=false;
    z_time _t_started;

    int timer_callback(void*);
    Timer* _timer=0;
    z_file_out _record_file;
    z_string _record_file_fullname;
    std::mutex _mutex_tags;

    z_obj_map<RfidTag> _tags;
    //PROPS
    int _peak_window_ms = 500;
    int _min_rssi = 70;
    int _purge_ms = 500;
    int _min_split_time_s = 1;
    int _filter_epc = true;
    const int _tag_cleanup_timer_ms = 1000;
    int _tag_missing_cleanup_ms = 5000;
    bool _print_reads=true;
    bool _beep=true;
    bool _single_tag_test=true;
    bool _write_to_file=false;
    z_string _test_tag="0082";
    //bool _write_to_file=true;
    bool _recording=false;
    z_status _start_new_file();
    z_status _close_copy_file();

public:
    App0();
    z_string _file_path_record = "/zs/reader/reads";

    z_string _record_file_name = "record_live.csv";

    bool is_reading() { return _reading; }
    bool is_recording() { return _recording; }
    virtual z_status open();
    virtual z_status close();
    virtual z_status run();
    virtual z_status shutdown();
    virtual z_status setup_reader_live(z_json_obj &settings);

    virtual z_status remote_quit();
    virtual z_status stop();
    virtual z_status start();
    z_string createJsonStatus(int status, ctext msg,bool ack);


    virtual bool callbackRead(RfidRead* r);
    virtual bool callbackQueueEmpty();

    z_status initialize();

    int add_json_status(z_json_stream &js);

};


ZMETA_DECL(App0) {

    ZPROP(_record_file_name);
    ZPROP(_file_path_record);
    //ZPROP(_file_path_complete);
    ZPROP(_beep);
    ZPROP(_peak_window_ms);
    ZPROP(_tag_missing_cleanup_ms);
    ZPROP(_min_split_time_s);
    ZPROP(_min_rssi);
    ZPROP(_test_tag);
    ZPROP(_filter_epc);
    ZPROP(_single_tag_test);
    ZPROP(_recording);
    //ZPROP(_broadcast);
    ZPROP(_print_reads);
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

}

#endif //ZIPOSOFT_APP_H