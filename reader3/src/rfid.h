//
// Created by ac on 10/26/20.
//

#ifndef ZIPOSOFT_RFID_H
#define ZIPOSOFT_RFID_H
#include "pch.h"
#include "epc.h"
#include "timers.h"

class RfidTag
{
public:
    RfidTag()
    {
    }
    U64   _last_time_seen = 0;
    U8 _last_rssi=0;
    int _count = 0;
    bool _counted=false;
};
class RfidRead {
public:
    U8 _antNum=0;
    Epc _epc;
    U8 _rssi=0;
    U64  _time_stamp;
    U32 _index=0;
public:
    bool _recorded=false;

    void getEpcString(z_string& s);
    U64  get_ms_epoch()
    {
        return _time_stamp;
    }
    void getJson(z_string& s);
    void getJsonStream(z_json_stream& s);
    RfidRead(U32 index,U8 antnum,U8 rssi,U8* epc,size_t epc_len,U64 ts)
    {
        _index=index;
        _antNum=antnum;
        _rssi=rssi;
        _epc.set_bytes(epc,epc_len);
        _time_stamp=ts;


    }
    RfidRead(U32 index,U8 antnum,U8 rssi,const z_string& epc_str,U64 ts)
    {
        _antNum=antnum;
        _rssi=rssi;
        _epc.setFromHexString(epc_str);
        _index=index;
        _time_stamp=ts;

    }
};
class RfidReadConsumer {
public:
    virtual bool callbackRead(RfidRead* r)
    {
        zout << r->_epc << "\n";
        return false;
    }
    virtual bool callbackQueueEmpty(){
        return true;
    }
};

typedef struct {
    U8 QValue;
    U8 session;
    U8 target;
    U8 ant;
    U8 freq;
    U8 power;
    U8 pause_time;
} rfid_setup_t;

class RfidReader
{
    friend z_factory_t<RfidReader>;

    std::set<RfidReadConsumer*> _consumers;
    bool _reading=false;
    U64 _indexReads=0;
    z_safe_queue<RfidRead*> _queue_reads;
    std::deque<RfidRead*> _queue_reads_all;
    std::mutex _queue_reads_all_mutex;

    Timer* _stat_timer=0;
    int stat_timer_callback(void* context);

protected:
    bool _open=false;
    bool _debug_reads=false;
	U64 _total_bytes_read=0;

    U32 _queue_max_depth=1000;
    z_time _ts_reading_started;
    virtual z_status _read_start()  {   return zs_ok;  }
    virtual z_status _read_stop()  {   return zs_ok;  }
    virtual z_status _hw_open()  {   return zs_ok;  }
    virtual z_status _hw_close()  {   return zs_ok;  }
    void queueRead(U8 antnum,U8 rssi,U8* epc,size_t epc_len,U64 ts);


public:
    int _power=30;
    int _pause_read_time=0;
    int _session=0;
    int _qvalue=15;
    int _filter_time=5;
    int _antenna_config=0xf;
    int _antenna_mask=0xf;
    int _antenna_detected=0;

    int cached_read_count() {
        return _queue_reads_all.size();
    }
    RfidReader() {}
    virtual ~RfidReader();
    std::thread _thread_process_reads_thread;
    virtual z_status power_set(int val) {   return zs_ok;  }

    U32 get_queue_depth() { return _queue_reads.get_count(); }
    z_status get_reads_since(z_json_stream& s,U32 index,bool include_reads) ;

    void register_consumer(RfidReadConsumer* consumer);
    void remove_consumer(RfidReadConsumer* consumer);

    U32 getReadIndex(){return _indexReads;}
    bool isReading(){return _reading;}
    void process_reads_thread();
    z_status stop();
    z_status open();
    z_status close();
    z_status start();
    z_status dump_queue(int index);
    z_status remote_start(
            z_string timestamp,
            z_string id,
            z_string newfile,
            z_string num_ant,
            z_string powerlvl
    );
    virtual z_status setupParams(

            U8 antmask,
            U8 power,
            U8 session,
            U8 filterTime,
            U8 qValue

            )=0;
    virtual z_status readmode_get(){ return zs_not_implemented;}
    virtual z_status readmode_set(){ return zs_not_implemented;}
    virtual z_status info_get(){ return zs_not_implemented;}
    virtual z_status setup();
    int add_json_status(z_json_stream &js);

};


class RfidLogFile {
    int _write_count=0;
public:
    virtual bool callbackRead(RfidRead* r);
    virtual bool callbackQueueEmpty();
    z_file_out _record_file;
    z_string _record_file_name = "record_live.csv";
    z_obj_map<RfidTag> _tags;
    int _min_split_time = 6;

};

ZMETA_DECL(RfidReader)
{

    ZACT(open);
    ZACT(stop);
    ZACT(close);
    ZACT(start);
    ZACT(setup);
    ZCMD(dump_queue, ZFF_CMD_DEF, "dump_reads_since",
         ZPRM(int, index, 0, "index", ZFF_PARAM)
    );
    ZCMD(remote_start, ZFF_CMD_DEF, "start",
         ZPRM(z_string, timestamp, 0, "timestamp", ZFF_PARAM),
         ZPRM(z_string, id, "default", "id", ZFF_PARAM),
         ZPRM(z_string, newfile, 0, "newfile", ZFF_PARAM),
         ZPRM(z_string, num_ant, 2, "num_ant", ZFF_PARAM),
         ZPRM(z_string, powerlvl, 3000, "powerlvl", ZFF_PARAM)
    );
    //ZPROP(_fixed_lap_time_seconds);
    ZPROP(_debug_reads);
    ZPROP_X(_reading, "Running", ZFF_READ_ONLY, "Reader is started");
    ZCMD(power_set, ZFF_CMD_DEF, "power_set", ZPRM(int, val, 30, "val", ZFF_PARAM));
    ZPROP_X(_antenna_config, "antenna_config", ZFF_READ_ONLY, "Ant Config");
    ZPROP_X(_antenna_detected, "antenna_detected", ZFF_READ_ONLY, "Ant Detected");

    ZPROP(_queue_max_depth);
    ZPROP(_antenna_mask);
    ZPROP(_session);
    ZPROP(_pause_read_time);
    ZPROP(_power);
    ZPROP(_filter_time);
    ZPROP(_qvalue);
    ZSTAT(cached_read_count);
};
#endif //ZIPOSOFT_RFID_H
