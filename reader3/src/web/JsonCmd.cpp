//
// Created by ac on 7/27/21.
//

#include "JsonCmd.h"

#include "config.h"
#include "WebServer.h"

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
U64 us_last_ts=0;
int fn_get_pingpong(http_request r,z_string_map &vars)
{

    int count=vars.get_as("count",0);
    bool flash=vars.get_as("flash",false);
    count++;
    send_json_response(r,[count,flash](z_json_stream &js)
    {

        //if (flash) {             root.gpio.ledYellow.toggle();        }
        js.keyval_int("count",count);;
        if (count %1000 == 0) {
            printf("count=%d\n",count);
        }
        return HTTP_STATUS_OK;
    });
    return 200;
}

int fn_get_delay(http_request r,z_string_map &vars)
{

    int delay_ms=vars.get_as("ms",1000);

    delayed_request *req = new delayed_request();
            //req->r.hm->message = mg_strdup(r.hm);
    req->ts_expire=z_time_get_ticks_ms()+delay_ms;
    req->r=r;
    req->type=DELAYED_REQUEST_TEST;

    req->fn_complete=[](z_json_stream &js,size_t ctx,size_t ctx2) {
        js.keyval("status","deleay complete!!!!");

    };

    WEBSERV(r.c).push_delayed_request(req);

    return 200;
}


int fn_post_pong(http_request r,z_json_obj &o)
{
    const int iter_max=10000;
    static U64 us_start= 0;
    int count=o.get_int("count",0);

    if (count==0) {
        us_start= z_time_get_ticks_us();
    }

    z_status status=zs_bad_command;
    count++;

    send_json_response(r,[count](z_json_stream &js)
    {
        js.keyval_int("count",count);;
        if (count %1000 == 0) {
            printf("count=%d\n",count);
        }
        if (count==iter_max) {
            U64 us_end= z_time_get_ticks_us();
            U64 diff=us_end-us_start;
            double calls_per_sec=iter_max/((double)diff/1000000.0);
            printf("fn_post_pong  %d iters in %lu us, %lu per call, %lf calls per sec \n",iter_max,diff,diff/iter_max,calls_per_sec);

        }

        return HTTP_STATUS_OK;
    });


    return 200;
}


int fn_post_test500(http_request r,z_json_obj &o)
{
    z_string hex;
    bool found=o.get_str("epc",hex,"");
    bool overwrite=o.get_bool("overwrite",false);
    z_status status=zs_bad_command;


    send_json_response(r,[status](z_json_stream &js)
    {
        js.key_bool("success",status==zs_ok);
        js.keyval("error",zs_get_status_text(status));

        return (status?HTTP_STATUS_INTERNAL_SERVER_ERROR: HTTP_STATUS_OK);
    });


    return 200;
}