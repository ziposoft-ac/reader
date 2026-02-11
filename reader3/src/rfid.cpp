//
// Created by ac on 10/26/20.
//

#include "rfid.h"
#include "root.h"

ZMETA_DEFV(RfidReader);
#ifdef  ENABLE_PHASE
#define PHASE_FORMAT ",%d,%d,%d"
#else
#define PHASE_FORMAT

#endif
void RfidRead::getEpcString(z_string& s)
{

    //_epc.getAsciiString(s);
    _epc.getHexString(s);

}
int RfidReader::stat_timer_callback(void* context)
{
    static U64 last_bytes_read=0;
    static U64 last_call=0;

    U64 now=z_time::get_now_ms();


    static U64 last_read_index=0;
    if (last_read_index) {
        if (last_read_index != _indexReads) {
            ZDBG("Reads per second:%d (%d bytes)\n",_indexReads-last_read_index,_total_bytes_read-last_bytes_read);

        }
    }
    last_read_index=_indexReads;
    last_bytes_read=_total_bytes_read;

    // TODO disable for now
    return 0;
}


void RfidReader::process_reads_thread() {
    _queue_reads.wait_enable();
    U64 stats_report_last_index=0;
    U64 stats_report_last_bytes=0;
    U64 stats_report_last_ts=z_time::get_now_ms();

    int write_count = 0;
    U64 ts_last=0;
    try
    {
        ZT("process_reads_thread:");
        z_string line;
        while (1)
        {

            if (_queue_reads.get_count() == 0)
            {
                for(auto consumer :_consumers )
                {
                    consumer->callbackQueueEmpty();
                }
                //ZDBGS.flush();

            }


            RfidRead* r=0;



            bool running= _queue_reads.pop_wait(r);
            if (!running)
            {
                //quiting, exit
                ZT("process_reads_thread exit\n");

                return;
            }

#ifdef DEBUG
            if(_debug_reads)
            {

                U64 ts=r->_time_stamp - _ts_reading_started.get_t();
                U64 diff=r->_time_stamp - ts_last;
                ts_last=r->_time_stamp;
                z_string epc;
                r->_epc.getHexString(epc);
    #ifdef  ENABLE_PHASE

                //ZDBGS << r->_index<<'\t'<< ts << '\t'<< diff << '\t' <<queue<<'\t'<< r->_antNum  << '\t' << r->_rssi<< '\t' << r->_epc<<"\n";
                if (r->phase1 || r->phase2) {
                    I32 phase_diff=r->phase2;
                    //if (r->phase1>r->phase2)              phase_diff+=0x10000;
                    phase_diff=phase_diff-r->phase1;
                    if (phase_diff<-32768)
                        phase_diff+=0x10000;
                    if (phase_diff>32768)
                        phase_diff-=0x10000;

                    ZDBGS.format_append("%6u %6u.%03u %6u.%03u %2u %3u %s " PHASE_FORMAT ,r->_index,ts/1000,ts%1000,diff/1000,diff%1000,r->_antNum,r->_rssi,epc.c_str()
                    ,r->phase1,r->phase2,phase_diff

                    );
                    if ((phase_diff<3000)&&(phase_diff>-3000))
                        ZDBGS.format_append(" STILL\n");
                    if (phase_diff<-3000)
                        ZDBGS.format_append(" TOWARDS\n");
                    if (phase_diff>3000)
                        ZDBGS.format_append(" AWAY\n");
                    //ZDBGS << <<'\t'<< '\t' <<queue<<'\t'<< r->_antNum  << '\t' << r->_rssi<< '\t' << r->_epc<<"\n";
                }
                else
    #endif

                {
                    ZDBGS.format_append("%6u %6u.%03u %6u.%03u %2u %3u %s\n"  ,r->_index,ts/1000,ts%1000,diff/1000,diff%1000,r->_antNum,r->_rssi,epc.c_str());

                }


            }
            if (_read_stats) {
                U64 now=z_time::get_now_ms();
                if (stats_report_last_index != _indexReads) {
                    U64 elap=now - stats_report_last_ts;
                    if (elap> 1000) {
                        U64 reads=(_indexReads-stats_report_last_index)*1000;
                        U64 bytes=(_total_bytes_read-stats_report_last_bytes)*1000;
                        ZDBG("Reads per second:%d (%d bytes)\n",reads/elap,bytes/elap);
                        stats_report_last_bytes=_total_bytes_read;
                        stats_report_last_index=_indexReads;
                        stats_report_last_ts=now;
                    }

                }


            }
#endif
            for(auto consumer :_consumers )
            {
                consumer->callbackRead(r);
            }

            //delete read;
            sched_yield();
        }

    }
    catch (std::exception& e)
    {
        printf("\nexception :: %s\n", e.what());
    }
}


RfidReader::~RfidReader() noexcept {
    close();
    for (auto r : _queue_reads_all) {
        delete r;
    }
}


z_status RfidReader::open()
{
    z_status status;
    if (_open)
        return zs_ok;

    _stat_timer=root.timerService.create_timer_t(this,&RfidReader::stat_timer_callback,0,1000    );
    status=_hw_open();
    if(zs_ok==status)
    {

        _thread_process_reads_thread = std::thread(&RfidReader::process_reads_thread, this);
        _open=true;
    }
    else {
        Z_ERROR_MSG(zs_io_error,"could not open reader");
    }
    status=_hw_init();
    if(zs_ok==status)
    {
    }
    else {
        Z_ERROR_MSG(zs_io_error,"hw_init failed in reader");
    }



    return status;


}
z_status RfidReader::start()
{
    if (_reading)
        return zs_ok;
    z_status status=open();
    if(status)
        return status;
    if(_antenna_config==0)
    {
        return Z_ERROR_MSG(zs_io_error,"NO ANTENNA CONFIGURED");
    }
    status= _read_start();
    _ts_reading_started.set_now();
    if(status==zs_ok) {

        root.gpio.ledRed.off();
        root.gpio.ledGreen.on();

        _reading=true;

    }
    return status;



}
z_status RfidReader::configure(
        const rfid_config_t& c
        ){
    _qvalue=c.qValue;
    _session=c.session;
    _antenna_mask=c.antMask;
    z_status status=freq_set(c.freqLow,c.freqHigh);
    if (status)
        return status;
    _profile=c.profile;
    _power=c.power;
    _pause_read_time=c.pauseTime;
    _filter_time=c.filterTime;
    return config_write();
}
z_status RfidReader::config_dump() {

    z_status status=config_read();

    if (status!=zs_ok)
        return status;


    printf("ReadPauseTime=%d\n", _pause_read_time);
    printf("Antenna=%b\n", _antenna_detected);
    printf("FilterTime=%d\n", _filter_time);
    printf("QValue=%02x\n", _qvalue);
    printf("Session=%02x\n", _session);
    printf("Power=%d\n", _power);
    printf("Write Power=%d\n", _write_power);
    return zs_ok;
}


void RfidReader::queueRead(U8 antnum,U8 rssi,U8* epc,size_t epc_len,U64 ts
    #ifdef  ENABLE_PHASE
    ,int16_t phase1,int16_t phase2
    #endif
    )
{
	std::unique_lock<std::mutex> mlock(_queue_reads_all_mutex);
    _indexReads++;
    RfidRead* r=new RfidRead(_indexReads,antnum,rssi,(U8*)epc,epc_len,ts);
#ifdef  ENABLE_PHASE
    r->phase1=phase1;
    r->phase2=phase2;
#endif
    _queue_reads.push(r);
    _queue_reads_all.push_front(r);

    //std::unique_lock<std::mutex> mlock(_queue_reads_all_mutex);

    while (_queue_reads_all.size()>_queue_max_depth) {
        RfidRead* old=_queue_reads_all.back();
        z_string s;
        old->getEpcString(s);
        //ZDBG("DELETING OLD READ: %d %s\n",old->_index,s.c_str());
        _queue_reads_all.pop_back();
        delete old;
    }

}
z_status RfidReader::dump_queue(
        int since_index
)
{
	std::unique_lock<std::mutex> mlock(_queue_reads_all_mutex);
    bool complete=(since_index==0);
    int count=0;
    ZLOG("Queue size=%d\n",_queue_reads_all.size());
    ZLOG("_indexReads=%d\n",_indexReads);
    for (auto r : _queue_reads_all) {
        if (r->_index <= since_index)

            continue;
        count++;
        r->dump();
    }

    return zs_ok;
}
#define KV(K,V)
void RfidRead::getJson(z_string& s)
{

    z_json_stream js(s);
    getJsonStream(js);

}
void RfidRead::dump()
{
    z_string epc;
    _epc.getHexString(epc);
    ZLOG("%d: %llu %d %d %s\n",_index,_time_stamp,_antNum,_rssi,epc.c_str());


}
void RfidRead::getJsonStream(z_json_stream& js)
{

    js.obj_start();
    //js.keyval("command","read");
    js.keyval_int("ant",_antNum);
    js.key_bool("recorded",_recorded);
    //ZLOG("status %d\n",_recorded);
    js.keyval_int("rssi",_rssi);
    js.keyval_int("index",_index);
    js.keyval_int("ts",_time_stamp);
    z_string epc;
    _epc.getHexString(epc);
    js.keyval("epc",epc);
    js.obj_end();

}
/*
*export interface LiveReads
{
complete: boolean;
last_index: number;
count: number;
list:RfidRead[];
}
*/
z_status RfidReader::get_reads_since(z_json_stream &js,U32 index,bool return_reads) {

    std::unique_lock<std::mutex> mlock(_queue_reads_all_mutex);
    bool complete=(index==0);
    int count=0;
    int diff=_indexReads-index;

    js.keyval_int("last_index",_indexReads);
    js.key_bool("complete",complete);
    if (return_reads) {
        js.key("list");
        js.array_start();

        for (auto i : _queue_reads_all) {
            if (i->_index <= index) //skip old ones
                break;
            i->getJsonStream(js);
            count++;

        }
        js.array_end();
    }

    js.keyval_int("count",count);
    js.keyval_int("diff",diff);
    if (diff>0) {
        //ZDBG("get reads since %d, diff=%d count=%d return_reads=%d\n",index,diff,count,return_reads);

    }
    return zs_ok;

}

z_status RfidReader::json_config_get(z_json_stream &js) {


    js.key("reader_config");

    js.obj_start();

    z_status status=config_read();
    if (status==zs_ok) {
        js.keyval_int("antMask",_antenna_mask);
        js.keyval_int("qValue",_qvalue);
        js.keyval_int("power",_power);
        js.keyval_int("freqLow",_freq_low);
        js.keyval_int("freqHigh",_freq_high);
        js.keyval_int("session",_session);
        js.keyval_int("pauseTime",_pause_read_time);
        js.keyval_int("filterTime",_filter_time);
        js.keyval_int("beepEnable",_filter_time);


        js.key_bool("valid",true);
        js.key_bool("reading",_reading);
        js.keyval_int("profile",_profile);
        js.keyval_int("pause_read_time",_pause_read_time);

    }
    else {
        js.key_bool("valid",false);

    }

    js.obj_end();

    return status;
}

z_status RfidReader::json_status_get(z_json_stream &js) {

    js.key("reader_status");

    js.obj_start();
    js.key_bool("reading",_reading);
    js.keyval_int("ant_config",_antenna_config);
    js.keyval_int("ant_detected",_antenna_detected);
    js.obj_end();

    return zs_ok;
}

z_status RfidReader::stop()
{
    // in case we powered up reading already
    if (!_open)  return zs_ok;
    //if (!_reading)  return zs_ok;

    ZTF;
    _read_stop();
    _reading = false;
    root.gpio.ledRed.on();
    root.gpio.ledGreen.off();
    return zs_ok;

}
z_status RfidReader::close()
{
    if(_open)
    {
        stop();
        // release thread waiting on queue
        _queue_reads.wait_disable();
        ZTF;
        _hw_close();
        if (_thread_process_reads_thread.joinable())
            _thread_process_reads_thread.join();
        _open=false;
    }
    return zs_ok;


}

void RfidReader::register_consumer(RfidReadConsumer *consumer) {
    //todo error checking

    if(_consumers.find(consumer)==_consumers.end()) {

        _consumers.insert(consumer);
        //ZLOG("Registered consumer for RfidReader\n");
    }

    //_consumers.add(consumer);
}
void RfidReader::remove_consumer(RfidReadConsumer *consumer)
{
    if(_consumers.find(consumer)!=_consumers.end())
        _consumers.erase(consumer);
    //if(_consumers.contains(consumer))

}
