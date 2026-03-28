//
// Created by ac on 7/27/21.
//

#include "JsonCmd.h"

#include "root.h"
#include "config.h"

#include "zipolib/z_time.h"
#include "zipolib/http_status.h"


#define CMD_POST(_NAME_) int fn_post_##_NAME_(http_request req,z_json_obj &jin);
#define CMD_GET(_NAME_) int fn_get_##_NAME_(http_request req,z_string_map &vars);

// declare functions
CMDS;

#undef CMD_GET
#undef CMD_POST
#define CMD_GET(_NAME_) { "/"#_NAME_ ,REQUEST_GET, (size_t) fn_get_##_NAME_ },
#define CMD_POST(_NAME_) { "/"#_NAME_ ,REQUEST_POST,(size_t)fn_post_##_NAME_ },

cmd_entry_t cmd_list[]={
    CMDS
};

const size_t cmd_list_size = (sizeof(cmd_list)/sizeof(cmd_entry_t));

void send_command_response(http_request r,z_status zstatus,http_status_t http_status,const z_string& msg) {
    send_json_response(r,[zstatus,http_status,msg](z_json_stream &js)
    {
        js.keyval("msg",msg);
        js.keyval_int("result",zstatus);
        return http_status;

    });


}
/*
http_status_t send_status(http_request r,http_status_t status=HTTP_STATUS_OK,ctext message=nullptr) {
    send_json_response(r,[status,message](z_json_stream &js)
    {
        js.key_bool("reading",root.getReader().isReading());
        if (message)
            js.keyval("msg", message);
        return status;

    });

    return status;
}*/
http_status_t send_app0_status(http_request r,http_status_t status=HTTP_STATUS_OK,ctext message=nullptr) {
    send_json_response(r,[status,message](z_json_stream &js)
    {
        root.app0.add_json_status(js);
        if (message)
            js.keyval("msg", message);
        return status;

    });

    return status;
}
http_status_t send_rfid_status(http_request r,http_status_t status=HTTP_STATUS_OK,ctext message=nullptr) {
    send_json_response(r,[status,message](z_json_stream &js)
    {
        root.getReader().add_json_config(js);
        root.getReader().add_json_status(js);
        if (message)
            js.keyval("msg", message);
        return status;

    });

    return status;
}
http_status_t send_full_status(http_request r,http_status_t status=HTTP_STATUS_OK,ctext message=nullptr) {
    send_json_response(r,[status,message](z_json_stream &js)
    {
        root.getReader().add_json_config(js);
        root.getReader().add_json_status(js);
        root.app0.add_json_status(js);

        if (message)
            js.keyval("msg", message);
        return status;

    });

    return status;
}
int fn_get_status(http_request r,z_string_map &vars)
{
    return send_full_status(r);
}

int fn_get_gpio(http_request r,z_string_map &vars)
{
    send_json_response(r,[](z_json_stream &js)
    {
        z_status status=root.gpio.json_config_get(js);
        return (status?HTTP_STATUS_SERVICE_UNAVAILABLE: HTTP_STATUS_OK);
    });
    return 200;
}
int fn_post_gpio(http_request r,z_json_obj &o)
{

    z_status status=root.gpio.led_json_config_set(o);
    send_json_response(r,[status](z_json_stream &js)
    {
        root.gpio.json_config_get(js);
        return (status?HTTP_STATUS_INTERNAL_SERVER_ERROR: HTTP_STATUS_OK);
    });


    return 200;
}
int fn_get_config(http_request r,z_string_map &vars)
{
    z_status status=root.getReader().config_read();
    send_full_status(r);

    return 200;
}

int fn_get_pingpong(http_request r,z_string_map &vars)
{

    int count=vars.get_as("count",0);
    bool flash=vars.get_as("flash",false);
    count++;
    send_json_response(r,[count,flash](z_json_stream &js)
    {
        if (flash) {
            root.gpio.g27.toggle();
        }
        js.keyval_int("count",count);;
        return HTTP_STATUS_OK;
    });
    return 200;
}
int fn_get_beep(http_request r,z_string_map &vars)
{

    if (root.gpio.beepPwm._enabled) {
        root.gpio.beepPwm.toneRise();
    }
    else {
        int dur=vars.get_as("dur",25);
        int count=vars.get_as("count",2);
        while (count--)
            root.gpio.beeper.pushBeeps({{dur,25}});
    }

    send_json_response(r,[](z_json_stream &js)
    {

        return  HTTP_STATUS_OK;
    });
    return 200;
}

int fn_post_config(http_request r,z_json_obj &o)
{
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
    z_status s=root.getReader().configure(cfg);

    if (s==zs_ok) {
        send_json_response(r,[](z_json_stream &js)
        {
            root.getReader().json_config_get(js);
            return HTTP_STATUS_OK;


        });
        return HTTP_STATUS_OK;

    }
    else {
        send_command_response(r,s,HTTP_STATUS_INTERNAL_SERVER_ERROR,"failed");

        return HTTP_STATUS_INTERNAL_SERVER_ERROR;

    }
}


int fn_post_start_raw(http_request r,z_json_obj &o)
{
    if (root.getReader().isReading()) {
        return send_rfid_status(r,HTTP_STATUS_SERVICE_UNAVAILABLE,"already reading");
    }
    if (root.getReader().start()) {
        return send_rfid_status(r,HTTP_STATUS_SERVICE_UNAVAILABLE,"start command failed");

    }


    return send_rfid_status(r);
}
int fn_post_stop_raw(http_request r,z_json_obj &o)
{    if (!root.getReader().isReading()) {
        return send_rfid_status(r,HTTP_STATUS_SERVICE_UNAVAILABLE,"is not reading");

    }
    root.getReader().stop();

    send_rfid_status(r);

    return HTTP_STATUS_OK;
}
int fn_post_start_app0(http_request r,z_json_obj &o)
{
    if (root.getReader().isReading()) {
        return send_rfid_status(r,HTTP_STATUS_SERVICE_UNAVAILABLE,"already reading");
    }
    z_string path=o.get_str_def("path",default_record_path);
    bool record_raw=false;
    o.get_bool("raw",record_raw,false);

    root.app0.start();

    send_full_status(r);
    return HTTP_STATUS_OK;
}
int fn_post_stop_app0(http_request r,z_json_obj &o)
{

    root.app0.stop();

    send_full_status(r);

    return HTTP_STATUS_OK;
}
int fn_post_beepon(http_request r,z_json_obj &o)
{
    bool beep;

    o.get_bool("beep",beep,false);
    root.gpio.readBeep.off();
    ZLOG("fn_post_beepon");
    send_rfid_status(r);
    return HTTP_STATUS_OK;
}

int fn_post_beepoff(http_request r,z_json_obj &o)
{
    bool beep;

    o.get_bool("beep",beep,false);
        root.gpio.readBeep.on();
    ZLOG("fn_post_beepoff");

    send_rfid_status(r);
    return HTTP_STATUS_OK;
}
int fn_get_reads_raw(http_request r,z_string_map &vars)
{
    RfidReader &reader = root.getReader();

    I64 fromIndex=vars.get_as("fromIndex",0);
    bool return_reads=vars.get_as("return_reads",false);
    bool debug=vars.get_as("debug",true);
    if (fromIndex>reader.getReadIndex()) {
        fromIndex=0;
        ZDBG("R#%d:RAW requested index %d greater than current, using 0\n",r.index,fromIndex);
    }

    if (reader.getReadIndex()==fromIndex) {
        delayed_request *req = new delayed_request();
        req->r=r;
        ZDBG("R#%d: RAW (%s) index %d matches, wait for reads\n",r.index,(return_reads?"reads":"stat"),fromIndex);

        req->ts_expire=z_time_get_ticks()+WAIT_FOR_NEW_READS_TIMEOUT;
        req->ctx1=fromIndex;
        req->type=DELAYED_REQUEST_READS_RAW;
        req->ctx2=return_reads;
        req->fn_complete=[](z_json_stream &js,size_t fromIndex,size_t return_reads) {
            //ZDBG("R#:(%s) delay complete %d to %d\n",(return_reads?"reads":"stat"),last_index,root.getReader().getReadIndex());

            js.set_pretty_print(true);
            RfidReader &reader = root.getReader();
            reader.get_reads_since(js, fromIndex,return_reads);
        };
        WEBSERV(r.c).push_delayed_request(req);
        return 0;
    }
    send_json_response(r,[r,fromIndex,return_reads](z_json_stream &js)
    {
        ZDBG("R#%d:RAW(%s) returning reads from %d to %d\n",r.index,(return_reads?"reads":"stat"),fromIndex,root.getReader().getReadIndex());

        js.set_pretty_print(true);
        RfidReader &reader = root.getReader();
            reader.get_reads_since(js, fromIndex,return_reads);

        //root.app0.add_json_status(js);
        return HTTP_STATUS_OK;

    });


    return 200;
}
int fn_get_reads_filtered(http_request r,z_string_map &vars)
{
    App0& app=root.app0;

    U64 fromIndex=vars.get_as("fromIndex",(U64)0);

    bool debug=vars.get_as("debug",true);
    if (fromIndex>app.getLastWriteTimestamp()) {
        fromIndex=0;
        ZDBG("R#%d: FLTR requested index %llu greater than current, using 0\n",r.index,fromIndex);
    }

    if (app.getLastWriteTimestamp()==fromIndex) {
        delayed_request *req = new delayed_request();
        req->r=r;
        //ZDBG("R#%d: FLTRindex %d matches, wait for reads\n",r.index,fromIndex);
        req->type=DELAYED_REQUEST_READS_FILTERED;

        req->ts_expire=z_time_get_ticks()+WAIT_FOR_NEW_READS_TIMEOUT;
        req->ctx1=fromIndex;
        req->ctx2=0; //unused
        req->fn_complete=[](z_json_stream &js,size_t fromIndex,size_t unused) {
            //ZDBG("R#:FLTR delay complete %llu to %llu\n",fromIndex,root.app0.getLastWriteTimestamp());
            root.app0.add_json_status(js);
        };
        WEBSERV(r.c).push_delayed_request(req);
        return 0;
    }
    send_json_response(r,[r,fromIndex](z_json_stream &js)
    {
        ZDBG("R#%d:FLTR sending immediate %llu to %llu\n",r.index,fromIndex,root.app0.getLastWriteTimestamp());

        root.app0.add_json_status(js);
        return HTTP_STATUS_OK;

    });


    return 200;
}

int fn_get_delay(http_request r,z_string_map &vars)
{
    delayed_request *req = new delayed_request();
            //req->r.hm->message = mg_strdup(r.hm);
    req->ts_expire=z_time_get_ticks()+5000;
    req->r=r;
    req->type=DELAYED_REQUEST_TEST;

    req->fn_complete=[](z_json_stream &js,size_t ctx,size_t ctx2) {
        js.keyval("status","deleay complete!!!!");

    };

    WEBSERV(r.c).push_delayed_request(req);

    return 200;
}
int fn_set_config(http_request r,z_string_map &vars)
{
    send_json_response(r,[](z_json_stream &js)
    {
        js.keyval("status","yeah boy!");
        return HTTP_STATUS_OK;
    });
    return 200;
}


int fn_post(z_json_obj &jin,z_json_stream &jout,z_string msg)
{

    return 0;
}
