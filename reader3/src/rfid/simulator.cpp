
//
// Created by ac on 8/25/21.
//

#include "simulator.h"


ZMETA(RfidSimulator)
{
    ZBASE(RfidReader);
    ZPROP(_source_file);
    ZPROP(_max_interval);
    ZPROP(_seq_max);
    ZPROP(_interval);
    ZPROP(_file_reads_delay);
    ZPROP(_mode);
    ZACT(setRandomMode);
    ZACT(setFileMode);
    ZACT(setManMode);

    ZCMD(loadFile, ZFF_CMD_DEF, "loadFile",
     ZPRM(z_string, fn, "raw_reads.csv", "raw reads input file", ZFF_PARAM)
);
    ZCMD(manRead, ZFF_CMD_DEF, "manRead",
         ZPRM(z_string, epc, "0001", "epc", ZFF_PARAM),
         ZPRM(int, rssi, 50, "rssi", ZFF_PARAM),
         ZPRM(int, ant, 1, "ant", ZFF_PARAM)
    );
    ZCMD(setSeqMode, ZFF_CMD_DEF, "setSeqMode",
     ZPRM(int, max, 0, "max", ZFF_PARAM),
     ZPRM(int, interval_ms, 0, "interval_ms", ZFF_PARAM)
    );
    ZCMD(burstSeq, ZFF_CMD_DEF, "burstSeq",
     ZPRM(int, count, 5, "count", ZFF_PARAM),
     ZPRM(int, interval_ms, 1, "interval_ms", ZFF_PARAM)
    );

};


z_status RfidSimulator::setupParams(

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

z_status RfidSimulator::burstSeq(int count,int interval_ms) {
    stop();
    _seq_max=getReadIndex()+count;
    if (interval_ms>0)
        _interval=interval_ms;

    _mode=MODE_SEQ;
    _read_start();
    return zs_ok;
}

z_status RfidSimulator::_read_start() {
    ZTF;
    if(!_timer)
        _timer=gTimerService.create_timer_t(this,&RfidSimulator::timer_callback,0    );
    if (_mode==MODE_FILE) {
        printf("Loading file: %s\n ",_source_file.c_str());
        z_parse_csv_file csv;
        z_string path=   _source_file;
        z_status  status=csv.ParseFileData(path,_data);

        if (status!=zs_ok) {
            return Z_ERROR(status);
        }
        _index=0;
        /*
        if(!_timer)
            _timer=gTimerService.create_timer_t(this,&RfidSimulator::timer_callback_file,0    );
        _timer->start(_interval);
        return zs_ok;
        */
        _timer->start_ms_reset(1);

    }
    if (_mode==MODE_SEQ) {
        _timer->start_ms_reset(1);

    }

    return zs_ok;
}
int RfidSimulator::timer_callback(void *p) {
    if (_mode==MODE_FILE) {
        return timer_callback_file(p);
    }
    Epc epc;
    epc.set_bcd_from_int(_index);
    if (_mode==MODE_RANDOM) {
        U8 d[Epc::_max_len];

        size_t len=rand()%Epc::_max_len;
        for (size_t i=0;i<len;i++) {
            d[i]=rand()%256;
        }
        epc.set_bytes(d,len);
    }
    if (_mode==MODE_SEQ) {
        epc.set_bcd_from_int(_index);
    }
    queueRead(1,54,epc.get_data(),epc.get_len(),z_time_get_ticks_ms()
#ifdef  ENABLE_PHASE
        ,0,0
#endif
        );
    if (_seq_max) {
        if (_index++ >=_seq_max)
            return 0;
    }

    return _interval;

}
z_status RfidSimulator::_read_stop() {
    ZTF;

    if(_timer)
        _timer->stop();

    return zs_ok;

}
z_status RfidSimulator::manRead(z_string hex,int rssi,int ant) {
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

z_status RfidSimulator::loadFile(z_string fn) {


    return zs_ok;
}

int RfidSimulator::timer_callback_file(void *) {
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
