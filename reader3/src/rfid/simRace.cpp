
//
// Created by ac on 8/25/21.
//

#include "simRace.h"

#include <cmath>


ZMETA(RfidSimRace) {
    ZBASE(RfidReader);
    ZPROP(_source_file);
    ZPROP(_max_interval);
    ZPROP(_seq_max);
    ZPROP(_interval);
    ZPROP(_file_reads_delay);
    ZOBJ(_race);
    ZACT(raceStart);
    ZACT(raceStop);
    ZCMD(manRead, ZFF_CMD_DEF, "manRead",
         ZPRM(z_string, epc, "0001", "epc", ZFF_PARAM),
         ZPRM(int, rssi, 50, "rssi", ZFF_PARAM),
         ZPRM(int, ant, 1, "ant", ZFF_PARAM)
    );

};


ZMETA(SimRace)
{
    ZPROP(_num_laps);
    ZPROP(_num_runners);
    ZPROP(_lap_dist_meters);
    ZPROP(_speed_mpm_max);
    ZPROP(_speed_mpm_min);
    ZPROP(_read_win_pre);
    ZPROP(_read_win_post);
    ZPROP(_interval_ms);
    ZACT(createRunners);


};

RfidRead* SimRunner::getRead(U64 ms,SimRace* race) {
    double  mps=1609.344/( _speed_mpm*60);
    double position=mps*ms/1000;


    int win_start=race->_read_win_pre + race->_lap_dist_meters;
    int win_end=race->_read_win_post + race->_lap_dist_meters;
    double offset=std::fmod(position , race->_lap_dist_meters);
    if ((offset<win_start)&&(offset > race->_read_win_post) )
        return 0;


    if (offset > race->_read_win_post)
        offset=race->_lap_dist_meters-offset;
    int rssi= (int)(100-offset*2);

    ZDBG("%lld runner %d ,offset %4.2lf position: %4.2lf rssi=%d\n",ms,_bib,offset,position,rssi);
    int ant=1 << ((int)(offset*100)%8);
    race->_reader->queueRead(ant,rssi,(U8*)_epc.get_data(),_epc.get_len(),z_time_get_ticks_ms());


    return 0;

}


z_status SimRace::start(RfidSimRace* r) {

    createRunners();
    _start=z_time_get_ticks_ms();
    _reader=r;
    if (!_timer)
        _timer=CREATE_TIMER(SimRace::timer_callback );
    _timer->start_ms_reset(_interval_ms);

    return zs_ok;
}

z_status SimRace::stop() {
    if (_timer)
        _timer->stop();
    return zs_ok;

}

int SimRace::timer_callback(void *) {
    U64 now=z_time_get_ticks_ms();
    U64 diff=now-_start;
    for (auto r:_runners) {


        r->getRead(diff,this);


    }
    return _interval_ms;
}



z_status SimRace::createRunners() {

    SimRunner x;

    _runners.delete_all();
    for (int i=0;i<_num_runners;i++) {
        SimRunner* r=new SimRunner();
        r->_bib=i+1;
        r->_epc.set_bcd_from_int(r->_bib);
        double speed_range=(_speed_mpm_min-_speed_mpm_max);
        double speed=(double)_speed_mpm_max+  (double)(i*(_speed_mpm_min-_speed_mpm_max))/_num_runners;
        r->_speed_mpm=speed;

        ZDBG("runner %d, speed=%4.2lf\n",i,speed);

        _runners.add(r);



    }
    return zs_ok;
}


z_status RfidSimRace::setupParams(

        U8 ant, //0==auto
        U8 power,
        U8 session,
        U8 filterTime,
        U8 qValue

)
{
    _power=power;
    _session=session;
    _qvalue=qValue;
    _qvalue=qValue;
    _filter_time=filterTime;
    _antenna_enabled=ant;
    //_antenna_mask=ant;

    _antenna_detected=0xf;

    return zs_ok;
}


z_status RfidSimRace::_read_start() {
    ZTF;
    if(!_timer)
        _timer=CREATE_TIMER(RfidSimRace::timer_callback );
    _timer->start_ms_reset(1);
    _race.start(this);


    return zs_ok;
}
int RfidSimRace::timer_callback(void *p) {

    return _interval;

}
z_status RfidSimRace::_read_stop() {
    ZTF;

    if(_timer)
        _timer->stop();

    raceStop();
    return zs_ok;

}
z_status RfidSimRace::manRead(z_string hex,int rssi,int ant) {
    Epc epc;
    epc.setFromHexString(hex);
    U64 ts=z_time_get_ticks_ms();
    ZLOG("queueing read %llu %s\n",ts,hex.c_str());
    queueRead(ant,rssi,(U8*)epc.get_data(),epc.get_len(),ts
#ifdef  ENABLE_PHASE
        ,0,0
#endif

        );

    return zs_ok;

}


int RfidSimRace::timer_callback_file(void *) {
    if(_index>=_data.size()) {
        printf("File complete\n");
        return 0;

    }
    auto row=_data[_index];
    if(row.size()<3)
        return 0;

    U64 ts=row[1].get_u64_val();
    U8 ant=row[2].get_int_val();
    U8 rssi=row[3].get_int_val();
    if(!_time_offset)
    {
        _time_offset=z_time_get_ticks_ms();
        _time_offset=_time_offset-ts;
    }
    U64 adj_ts=ts+_time_offset;
    //zout<< ts<<"\n";
    //ZDBG("read %d %s\n",adj_ts,row[2].c_str());
    Epc epc;
    epc.setFromHexString(row[4]);

    queueRead(ant,rssi,epc.get_data(),epc.get_len(),adj_ts);
    _index++;
    if(_index>=_data.size()) {
        printf("File complete\n");
        return 0;
    }
    row=_data[_index];
    U64 delay=row[1].get_u64_val() - ts;

    if (_file_reads_delay)
        delay=_file_reads_delay;
    ZDBG("next read %d\n",delay);
    if(_max_interval)
        if(_max_interval<delay)
            delay=_max_interval;
    return delay;
}
