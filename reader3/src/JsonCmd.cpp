//
// Created by ac on 7/27/21.
//

#include "JsonCmd.h"

#include "root.h"
#include "config.h"

#include "zipolib/z_time.h"


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

void send_command_response(http_request r,z_status status,const z_string& msg) {
    send_json_response(r,[status,msg](z_json_stream &js)
    {
        js.keyval("msg",msg);
        js.keyval_int("result",status);


    });


}
void send_status(http_request r) {
    send_json_response(r,[](z_json_stream &js)
    {
        js.key_bool("reading",root.getReader().isReading());


    });


}

int fn_get_config(http_request r,z_string_map &vars)
{


    send_json_response(r,[](z_json_stream &js)
    {
        root.getReader().get_json_config(js);


    });


    return 200;
}

int fn_post_configure(http_request r,z_json_obj &o)
{
    rfid_config_t cfg;

    o.print(stdout_json);
    cfg.qValue=o.get_int("qValue",5);
    cfg.session=o.get_int("session",1);
    cfg.power=o.get_int("power",30);
    cfg.pauseTime=o.get_int("pauseTime",0);
    cfg.antMask=o.get_int("antMask",0xf);
    cfg.freqLow=o.get_int("freqLow",0);
    cfg.freqHigh=o.get_int("freqHigh",3);
    cfg.filterTime=o.get_int("filterTime",0);
    z_status s=root.getReader().configure(cfg);


    send_command_response(r,s,"ok");
    return 200;
}
int fn_post_start(http_request r,z_json_obj &o)
{
    root.app.start();

    send_status(r);
    return 200;
}
int fn_post_stop(http_request r,z_json_obj &o)
{
    root.app.stop();

    send_status(r);

    return 200;
}



int fn_get_reads(http_request r,z_string_map &vars)
{
    RfidReader &reader = root.getReader();

    I64 ofs=vars.get_as("last_index",0);
    bool statusOnly=vars.get_as("status_only",false);
    if (ofs>reader.getReadIndex()) {
        ofs=0;
    }

    if (reader.getReadIndex()==ofs) {
        delayed_request *req = new delayed_request();
        req->r=r;

        req->ts_expire=z_time_get_ticks()+WAIT_FOR_NEW_READS_TIMEOUT;
        req->ctx1=ofs;
        req->ctx2=statusOnly;
        req->fn_complete=[](z_json_stream &js,size_t ctx1,size_t ctx2) {
            js.set_pretty_print(true);
            RfidReader &reader = root.getReader();
            reader.get_reads_since(js, ctx1,ctx2);
            if (ctx2)
                root.app.add_json_status(js);
        };
        WEBSERV(r.c)._outstanding_reqs.push_back(req);
        return 0;
    }
    send_json_response(r,[ofs,statusOnly](z_json_stream &js)
    {
        js.set_pretty_print(true);
        RfidReader &reader = root.getReader();
            reader.get_reads_since(js, ofs,statusOnly);

        if (statusOnly) {
            root.app.add_json_status(js);
        }
    });


    return 200;
}

int fn_get_delay(http_request r,z_string_map &vars)
{
    delayed_request *req = new delayed_request();
            //req->r.hm->message = mg_strdup(r.hm);
    req->ts_expire=z_time_get_ticks()+5000;
    req->r=r;
    req->fn_complete=[](z_json_stream &js,size_t ctx,size_t ctx2) {
        js.keyval("status","deleay complete!!!!");

    };

    WEBSERV(r.c)._outstanding_reqs.push_back(req);


    return 200;
}
int fn_set_config(http_request r,z_string_map &vars)
{
    send_json_response(r,[](z_json_stream &js)
    {
        js.keyval("status","yeah boy!");
    });


    return 200;
}
int fn_get_status(http_request r,z_string_map &vars)
{
    send_json_response(r,[](z_json_stream &js)
    {
        js.keyval("status","yeah boy!");
    });


    return 200;
}

int fn_get_test(http_request r,z_string_map &vars)
{
    send_json_response(r,[](z_json_stream &js)
    {
        js.keyval("status","test boy!");
    });


    return 200;
}
int fn_get_test(z_string_map &vars,z_json_stream &jo,z_string& msg)
{
    jo.keyval("status","testing!!");
    return 200;
}
int fn_post(z_json_obj &jin,z_json_stream &jout,z_string msg)
{

    return 0;
}
#if 0
int callback_rx(ctext in, int len,
    ctext* p_status_msg // TODO, bad bad bad

    ) {
    zp_text_parser p;
    z_json_obj obj=p.parseJsonObj(in,len);
    z_string path= obj.get_str_def("path","./");
    z_status status=zs_bad_command;
    ctext status_msg=NULL;


    z_string cmd= "";
    obj.get_str_def("command","");

    ZDBG("command=%s\n",cmd.c_str());
    if(cmd=="quit")
        status=root.quit_notify();
    if(cmd=="shutdown")
        status=root.quit_notify();
    if(cmd=="program_epc")
    {
        status=zs_ok;

        z_string epc_str=obj.get_str("epc");
        Epc epc(epc_str);
        root.cfmu804.program_epc(epc);
        RfidRead* r=root.cfmu804.read_single();
        if(!r)
        {
            r=new RfidRead(0,0,0,0,0,0);
            status=zs_not_open;
            status_msg="could not read chip";
        }
        else
        {
            root.gpio.buzzer.toneRise();
        }
        z_string m;
        r->getJson(m);
        //write_socket(m);
        delete r;
    }
    if(cmd=="program_setup") {

        //status=root.cfmu804.program_setup();
    }
    if(cmd=="program_bcd")
    {
        status=zs_ok;

        int number=obj.get_int("number");
        bool overwrite=false;
        obj.get_bool("overwrite",overwrite,false);

        bool programmed=false;
        RfidRead* r=root.cfmu804.program_bcd(number,overwrite,programmed);
        if(!r)
        {
            r=new RfidRead(0,0,0,0,0,0);
            status=zs_not_open;
            status_msg="could not read chip";
        }
        else
        {
            if(programmed)
                root.gpio.buzzer.toneRise();
        }
        z_string m;
        r->getJson(m);
        //write_socket(m);
        delete r;
    }
    if(cmd=="readtest_start")
    {
       // status=root.app.testAnt.readTest(obj);

    }
    if(cmd=="readone")
    {
        status=zs_ok;

        RfidRead* r=root.cfmu804.read_single();
        if(!r)
        {
            r=new RfidRead(0,0,0,0,0,0);

            status=zs_not_open;
            status_msg="currently reading or not open";

        }
        else
        {
           // root.gpio.buzzer.toneRise();
        }
        z_string m;
        r->getJson(m);
        //write_socket(m);
        delete r;


    }
    if(cmd=="readtest_stop")
    {
        //status=root.app.testAnt.stop();
    }
    if(cmd=="startNewFile")
    {
        root.app.stop();
        root.app.setup_reader_live(obj);
        z_string path=obj.get_str("path");

       // root.app.start_new_file();
        root.app.start();
        status=zs_ok;
    }
    if(cmd=="continueFile")
    {
        root.app.setup_reader_live(obj);

        status=root.app.start();

    }

    if(cmd=="stopReading")
    {
        status=root.app.stop();
    }
    if(cmd=="getStatus")
    {
        status=zs_ok;
    }
    if(status==zs_bad_command)
    {
        status_msg=cmd;
        Z_WARN_MSG(zs_bad_command,"unknown command:",cmd.c_str());
    }
    if(!status_msg)
        status_msg=zs_get_status_text(status);

    *p_status_msg=status_msg;
    return status;
}

#endif