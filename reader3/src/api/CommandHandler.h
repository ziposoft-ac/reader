//
// Created by ac on 7/20/26.
//

#ifndef ZIPOSOFT_COMMANDHANDLER_H
#define ZIPOSOFT_COMMANDHANDLER_H
#include "pch.h"

#include "zipolib/z_status.h"
#include "zipolib/z_string.h"


class CommandHandler;
typedef z_status (CommandHandler::*cb_post_json_t)(z_json_obj &jin);
//typedef z_status (CommandHandler::*cb_post_json_reply_t)(z_json_obj &jin,z_json_stream &js);
typedef z_status (CommandHandler::*cb_get_json_t)(z_json_obj &jin,z_json_stream &js);


// Template alias creating a unique lambda type per template parameter T
//template <typename T> using TemplatedLambdaType = decltype([](T x) {    return x * 2;});
using cb_post_json_reply_t = std::function<z_status(z_json_obj &jin,z_json_stream &j)>;




class Command {

    public:
    z_string _name;
    Command(ctext cmd) {
        _name=cmd;
    };

    z_status raw_rx(CommandHandler* obj,const char* data,size_t len,z_string &return_buffer) {
        zp_text_parser p;
        z_json_obj json_in=p.parseJsonObj(data,len);
        z_string msg_out;
        z_json_stream json_out(return_buffer);
        json_out.obj_start();
        json_out.keyval_int("ts",z_time::get_now_ms());


        //post_json_reply(json_in,json_out);
        z_status status= post_json_reply(json_in,json_out);

        json_out.obj_end();

        return status;
    }

    cb_post_json_reply_t post_json_reply;

    union {
        //cb_post_json_t post_json;
        //cb_get_json_t get_json;
    } cb;;

    CommandHandler* _obj;

};


class CommandDelayed : public Command {

public:
   // z_obj_list<delayed_request> _outstanding_reqs;


};
class CommandHandler {
public:
    z_obj_map<Command,true> _map;
    std::mutex _map_mutex;
    z_status cmd_exists(ctext name) {
        Command *cmd=0;
        if (!_map.get(name,cmd))
            return zs_not_found;

        return zs_ok;
    }
    z_status callback_rx(ctext name,const char* data,size_t len,z_string &return_buffer) {
        Command *cmd=0;
        if (!_map.get(name,cmd))
            return zs_not_found;
        cmd->raw_rx(this,data,len,return_buffer);

        return zs_ok;
    }
    template <class SUBCLASS>z_status reg_fn(ctext name,

        z_status (SUBCLASS::*callback)(z_json_obj &jin,z_json_stream &jout)

        ) {
	    std::unique_lock<std::mutex> mlock(_map_mutex);

        Command *cmd=new Command(name);
        cmd->post_json_reply=[this,callback](z_json_obj &jin,z_json_stream &jout) {
            SUBCLASS* sub=(SUBCLASS*)this;
            return  (sub->*callback)(jin,jout);

        };
        if (!_map.add(name,cmd)) {
            delete cmd;
            return Z_ERROR(zs_already_exists);
        }
        return zs_ok;
    }


};



#endif //ZIPOSOFT_COMMANDHANDLER_H
