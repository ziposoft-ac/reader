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

int RfidService::getVisits(http_request req, z_string_map &vars, z_json_obj &jin, z_json_stream &jout) {

    I64 req_last_notify=vars.get_as<I64>("last_notify",0);
    bool return_reads=vars.get_as("return_reads",true);
    bool debug=vars.get_as("debug",true);
    I64 last_notify=_visits.getLastNotifyTimestamp();

   // ZDBG("requested %lld,  current,%lld \n",req_last_notify,last_notify);

    if (req_last_notify>last_notify) {
        ZDBG("RAW requested index %lld greater than current, using 0\n",req_last_notify);
        req_last_notify=0;

    }
    if (req_last_notify==last_notify) {
        //ZDBG("queueing req\n");

        _getGetVisitsCd->add_pending(req,WAIT_FOR_NEW_READS_TIMEOUT,
            [this,req_last_notify,return_reads](z_json_stream& js) {
                // this is bad, because each separate delayed request generates its own response data
                //  even though it is the same for all of them. but normally only have one anyway

                _visits.add_json_status(js);


        });
        return HTTP_STATUS_PROCESSING;
    }

    _visits.add_json_status(jout);

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
    RfidReader& r=getRfidReader();
    if (!r.isReading()) {
        r.config_read();
    }
    return json_status(jout);
}
int RfidService::json_status(z_json_stream& jout) {
    U64 us_end= z_time_get_ticks_us();
    jout.keyval_int("tsUs",us_end);

    getRfidReader().add_json_status(jout);
    _visits.add_json_status(jout);
    return CMD_SUCCESS;
}

int RfidService::get_raw_reads(z_string_map &params, z_json_stream &jout) {

    return -1;
}

int RfidService::post_config(z_json_obj& o,z_json_stream& jout) {

    z_status s=getRfidReader().json_config_set(o);

    if (s==zs_ok) {
        json_status(jout);
        return CMD_SUCCESS;

    }
    return CMD_FAILED;
}

int RfidService::post_start_stop_visits(z_json_obj& o,z_json_stream& jout) {
    bool start= o.get_bool("start",false);
    bool visit_running=_visits.is_recording();



    int ret=CMD_FAILED;
    ctext msg="Unexpected error";
    if (start) {
        if (visit_running)
            msg="already reading";
        else {
            if (_visits.start())
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
            if (_visits.stop())
                msg="start command failed";
            else {
                ret=CMD_SUCCESS;
                msg="Reader stopped";
            }
        }
    }
    jout.keyval("result",msg);
    return json_status(jout);

}
