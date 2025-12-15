//
// Created by ac on 8/25/21.
//

#ifndef ZIPOSOFT_SIMULATOR_H
#define ZIPOSOFT_SIMULATOR_H
#include "pch.h"
#include "serial.h"
#include "rfid.h"
#include "zipolib/csv_files.h"



#define MODE_MANUAL "manual"
#define MODE_FILE "file"
#define MODE_RANDOM "random"
#define MODE_SEQ "seq"

class RfidSimulator : public RfidReader,public z_parse_csv {
    int timer_callback_file(void*);
    int timer_callback(void*);

    Timer* _timer=0;
    std::vector<z_strlist> _data;
    int _index=0;
    U64 _time_offset=0;
public:
    int _max_interval=3000;
    int _interval=1000;
    int _seq_max=10;
    z_string _mode=MODE_SEQ;
    virtual z_status _read_start()  ;
    virtual z_status _read_stop()  ;
    z_string _source_file = "simulate.csv";
    virtual z_status setupParams(
            U8 antmask,
            U8 power,
            U8 session,
            U8 filterTime,
            U8 qValue
    );
    virtual z_status manRead(z_string epc,int ant) ;

    virtual z_status setRandomMode() {
        _mode=MODE_RANDOM;
        return zs_ok;
    }
    virtual z_status setFileMode() {
        _mode=MODE_FILE;
        return zs_ok;
    }
    virtual z_status setManMode() {
        _mode=MODE_MANUAL;
        return zs_ok;
    }
    virtual z_status burstSeq(int max,int interval_ms);


    virtual z_status setSeqMode(int max,int interval_ms) {
        stop();
        if (max>0)
            _seq_max=max;
        if (interval_ms>0)
            _interval=interval_ms;

        _mode=MODE_SEQ;
        return zs_ok;
    }
};


#endif //ZIPOSOFT_SIMULATOR_H
