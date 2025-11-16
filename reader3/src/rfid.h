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
    U8 qValue;
    /*
    *Session: 1 byte, the Session-value of the EPC tag inventory.
        0x00 – apply S0 as Session value;
        0x01 – apply S1 as Session value;
        0x02 – apply S2 as Session value;
        0x03 – apply S3 as Session value;
        0xff – apply reader smart configuration (only valid in EPC inventory).
     */
    U8 session;
    //U8 target;
    U8 antMask;
    /*
     * Set freq quadrants, 0-3
     * Full range: low=0 hi=3
     * Bottom half: low=0 hi=1
     * Top half : low=2 hi=3
 */
    U8 freqLow;
    U8 freqHigh;

    U8 power;

    /*
    *ReadPauseTime: 1 byte, time break between 2 real time inventories.
        0x00 – 10ms;
        0x01 – 20ms;
        0x02 – 30ms;
        0x03 – 50ms;
        0x04 – 100ms.
     */
    U8 pauseTime;
    /*
     *FliterTime: 1 byte, tag filtering time. The valid value of this parameter is 0 ~ 255, corresponds to (0 ~ 255)*1s.
     *In real time inventory, if reader detects a particular tag for more than 1 time,
     *reader will only upload tag information of this tag once within the pre-defined filtering time.
     *For FliterTime = 0, disable tag filtering function.
     */
    U8 filterTime;

} rfid_config_t;

const rfid_config_t rfid_config_heattest={
    5,0,0xf,0,3,30,0,0
};
const rfid_config_t rfid_config_program={
    5,0,1,0,3,10,0,0
};

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
    int _write_power=10;
    int _pause_read_time=0;
    int _session=0;
    int _qvalue=15;
    int _filter_time=5;
    int _antenna_config=0xf;
    int _antenna_mask=0xf;
    int _antenna_detected=0;
    int _freq_low=0;
    int _freq_high=3;
    int _inventory_offtime=500;

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
    virtual z_status configure(
            const rfid_config_t& config
            );
    virtual z_status config_write(){ return zs_not_implemented;}
    virtual z_status config_read(){ return zs_not_implemented;}
    virtual z_status config_dump();
    virtual z_status readmode_get(){ return zs_not_implemented;}
    virtual z_status readmode_set(){ return zs_not_implemented;}
    virtual z_status info_get(){ return zs_not_implemented;}
    int add_json_status(z_json_stream &js);
    void get_json_config(z_json_stream &js);
	virtual z_status freq_set(U8 low,U8 max){ return zs_not_implemented;}

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
    ZACT(config_write);
    ZACT(config_dump);
    ZACT(config_read);
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
    ZPROP(_write_power);
    ZPROP(_power);
    ZPROP(_filter_time);
    ZPROP(_qvalue);
    ZSTAT(cached_read_count);
};
#endif //ZIPOSOFT_RFID_H
