//
// Created by ac on 7/25/26.
//
#include "RfidService.h"
#include "zipolib/http_status.h"

int RfidService::getRawReads(http_request req, z_string_map &vars, z_json_obj &jin, z_json_stream &jout) {
    RfidReader &reader = getRfidReader();

    I64 fromIndex=vars.get_as("fromIndex",0);
    bool return_reads=vars.get_as("return_reads",true);
    bool debug=vars.get_as("debug",true);
    I64 currentReadIndex=reader.getReadIndex();
    if (fromIndex>currentReadIndex) {
        fromIndex=0;
        ZDBG("RAW requested index %d greater than current, using 0\n",fromIndex);
    }
    if (fromIndex==currentReadIndex) {
        //ZDBG("queueing req\n");

        _getRawReadsCd->add_pending(req,WAIT_FOR_NEW_READS_TIMEOUT,
            [this,fromIndex,return_reads](z_json_stream& js) {
                RfidReader &reader = getRfidReader();
                reader.get_reads_since(js, fromIndex,return_reads);

        });
        return HTTP_STATUS_PROCESSING;
    }

    reader.get_reads_since(jout, fromIndex,return_reads);

    return 0;




}



int RfidService::post_start_stop_raw(z_json_obj& o,z_json_stream& jout) {
    bool start= o.get_bool("start",false);
    bool is_reading=getRfidReader().isReading();
    int ret=CMD_FAILED;
    ctext msg="Unexpected error";
    if (start) {
        if (is_reading)
            msg="already reading";
        else {
            if (getRfidReader().start())
                msg="start command failed";
            else {
                ret=CMD_SUCCESS;
                msg="Reader started";
            }
        }
    }
    else {
        if (!is_reading)
            msg="not reading";
        else {
            if (getRfidReader().stop())
                msg="start command failed";
            else {
                ret=CMD_SUCCESS;
                msg="Reader stopped";
            }
        }
    }
    jout.keyval("result",msg);
    getRfidReader().add_json_status(jout);
    return CMD_SUCCESS;
}
int RfidService::get_status(z_string_map& params,z_json_stream& jout) {
    getRfidReader().add_json_status(jout);
    _visits.add_json_status(jout);
    return CMD_SUCCESS;
}

int RfidService::get_raw_reads(z_string_map &params, z_json_stream &jout) {

    return -1;
}

int RfidService::post_config(z_json_obj& o,z_json_stream& jout) {
    rfid_config_t cfg;
    ZDBG("Setting config\n");
    //o.print(stdout_json);
    cfg.qValue=o.get_int("qValue",5);
    cfg.session=o.get_int("session",1);
    cfg.power=o.get_int("power",30);
    cfg.pauseTime=o.get_int("pauseTime",0);
    cfg.antMask=o.get_int("antMask",0xf);
    cfg.freqLow=o.get_int("freqLow",0);
    cfg.freqHigh=o.get_int("freqHigh",3);
    cfg.filterTime=o.get_int("filterTime",0);
    cfg.profile=o.get_int("profile",1);
    z_status s=getRfidReader().configure(cfg);

    if (s==zs_ok) {
        getRfidReader().json_config_get(jout);
        return CMD_SUCCESS;

    }
    return CMD_FAILED;
}

int RfidService::post_start_stop_visits(z_json_obj& o,z_json_stream& jout) {
    bool start= o.get_bool("start",false);
    bool is_reading=getRfidReader().isReading();
    bool visit_running=_visits.is_recording();

    int ret=CMD_FAILED;
    ctext msg="Unexpected error";
    if (start) {
        if (visit_running)
            msg="already reading";
        else {
            if (getRfidReader().start())
                msg="start command failed";
            else {
                ret=CMD_SUCCESS;
                msg="Reader started";
            }
        }
    }
    else {
        if (!visit_running)
            msg="not running";
        else {
            if (getRfidReader().stop())
                msg="start command failed";
            else {
                ret=CMD_SUCCESS;
                msg="Reader stopped";
            }
        }
    }
    jout.keyval("result",msg);
    getRfidReader().add_json_status(jout);
    return CMD_SUCCESS;
}
