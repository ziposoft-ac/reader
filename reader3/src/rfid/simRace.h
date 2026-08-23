//
// Created by ac on 8/25/21.
//

#ifndef ZIPOSOFT_SIM_RACE_H
#define ZIPOSOFT_SIM_RACE_H
#include "pch.h"
#include "rfid.h"
#include "zipolib/csv_files.h"



class SimRace;


class SimRunner
{
public:
    Epc _epc;
    int _bib=1;
    double _speed_mpm=8;
    double _speed_mps;

    RfidRead* getRead(U64 ms,SimRace* race);
    U64 lastRead_ts=0;

    int _read_interval_max=80;


};

class RfidSimRace;
class SimRace
{
public:
    U64 _start;
    RfidSimRace* _reader;

    z_obj_vector<SimRunner> _runners;
    int _num_runners=10;
    int _num_laps=14;
    int _lap_dist_meters=980;
    int _lap_time_min;
    int _speed_mpm_max=4;
    int _speed_mpm_min=20;

    int _read_win_pre=-10;
    int _read_win_post=2;


    int _interval_ms=10;
    Timer* _timer=0;

    z_status start(RfidSimRace* r);
    z_status stop();

    int timer_callback(void*);
    z_status createRunners();
};


class RfidSimRace : public RfidReader,public z_parse_csv {
    int timer_callback_file(void*);
    int timer_callback(void*);

    Timer* _timer=0;
    std::vector<z_strlist> _data;
    int _index=0;
    U64 _time_offset=0;
    U64 _ts_last_read=0;
public:
    SimRace _race;
    RfidSimRace() {
        _antenna_detected=3;
    }
    int _max_interval=3000;
    int _interval=1000;
    int _seq_max=10;
    int _file_reads_delay=0;
    virtual z_status _read_start()  ;
    virtual z_status _read_stop()  ;
    virtual z_status _hw_open()  {   return zs_ok;  }
    virtual z_status _hw_init()  {   return zs_ok;  }
    virtual z_status _hw_close()  {   return _race.stop();  }
	virtual z_status ant_mask_set(int mask) {   return zs_ok;  }

    z_string _source_file = "simulate.csv";
    virtual z_status setupParams(
            U8 antmask,
            U8 power,
            U8 session,
            U8 filterTime,
            U8 qValue
    );
    virtual z_status manRead(z_string epc,int rssi,int ant) ;
    virtual z_status raceStart() {
        _race.start(this);
        return zs_ok;
    }
    virtual z_status raceStop() {
        _race.stop();
        return zs_ok;
    }
};


#endif //ZIPOSOFT_SIMULATOR_H
