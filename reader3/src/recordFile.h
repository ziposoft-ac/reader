//
// Created by ac on 2/2/26.
//

#ifndef ZIPOSOFT_RECORDFILE_H
#define ZIPOSOFT_RECORDFILE_H

#include "pch.h"
#include "rfid.h"

class RecordFile;

enum FilteredReadState {
    fr_type_arrived,
    fr_type_new_peak,
    fr_type_peaked,
    fr_type_departed,

};
class RfidTag
{
public:
    RfidTag(RfidRead* r,ctext epc_str)
    {

        _epc=epc_str;
    }
    z_time   _ts_first_time_seen ;
    z_time   _ts_last_time_seen ;
    z_time   _ts_time_logged ;
    z_time   _ts_rssi_high ;
    z_time   _ts_next_check_required;

    z_string _epc;
    U8 _last_rssi=0;
    U8 _rssi_high_logged=0;
    U8 _rssi_high=0;
    U8 _ant_mask=0;
    int _count = 0;
    int _count_hi = 0;
    int _index=0;
    void writeOut(z_stream& s,FilteredReadState type );
    FilteredReadState _state=fr_type_arrived;
    // return time to check next
    //
    z_time processRead (RfidRead* read,RfidReadConsumer& s);

    // return true if it can be removed
    bool processCheck (RfidReadConsumer& s,z_time now);
    bool isDeparted() {
        return _state==fr_type_departed;
    }

};

class RecordFile {

    z_string _path;
    z_string _name;
    z_string _type;
    z_string _fullpath;
    z_time _time_opened;
    z_string _time_opened_str;
    z_file_out _file;

    z_stream_multi _stream;

public:
    RecordFile() {

    }
    virtual ~RecordFile() {
        close_copy();
    }
    bool is_open() {

        return _file.is_open();
    }
    void flush() {
        _file.flush();
        ZDBGS.flush();
    }
    ctext getLiveFileName() {
        return _name;
    }
    z_status writeRfidRead(RfidRead* read,ctext epc);
    z_status write(RfidRead* read,ctext type);
    z_status open_new(ctext path,ctext type,z_time ts);
    z_status close_copy();
    z_stream& get_stream() {
        return _stream;
    }

};


#endif //ZIPOSOFT_RECORDFILE_H