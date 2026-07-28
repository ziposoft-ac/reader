//
// Created by ac on 7/20/26.
//

#ifndef ZIPOSOFT_COMMANDHANDLER_H
#define ZIPOSOFT_COMMANDHANDLER_H
#include <variant>
#include "util/timers.h"

#include "pch.h"
#include <concepts>
#include "api/MqServer.h"
#include "web/WebServer.h"

class CommandHandler;




#define CMD_FAILED -1
#define CMD_SUCCESS 0

class Command {
public:
    size_t _binary_size=0;
    z_string _name;
    Command() {

    }
    virtual ~Command() {}
    Command(ctext name) {
        _name = name;
    };
    void init(ctext name) {
        _name = name;
    };

    virtual int call_post_raw_data(ctext buffer, size_t len, z_string &ret);

    virtual int process_http_rx(http_request req, cmd_req_type type);
    virtual int callback_http(http_request req,z_string_map &vars,z_json_obj &jin, z_json_stream &jout) {
        return -1;
    }
    virtual z_status callback_mq(MqMsg* msg);

    virtual void process_http_close(u_long id) {}


    int process_mq(MqMsg *msg);
    virtual int invoke_callback(int x,int y) {
        return 0;
    }

};
template <class C, typename... Args> using callback_t = int (C::*)(Args...);
template <class C> using callback_simple_t = int (C::*)(int);
template <class C> using callback_post_json_t = int (C::*)(z_json_obj &jin, z_json_stream &j);
template <class C> using callback_get_t = int (C::*)(z_string_map &vars, z_json_stream &j);
template <class C> using callback_http_delayed_t = int (C::*)(http_request req,z_string_map &vars,z_json_obj &jin, z_json_stream &jout);
template <class C> using callback_mq_binary_t = int (C::*)(const char*);


template <typename C,typename F> class Command_t  : public Command {
public:
    F _func;
    C *_obj;
    Command_t(ctext name,C *handler,F func): Command(name) {
        _obj=handler;
        _func=func;
    }
    virtual int invoke_callback(int x,int y) {
        if constexpr (std::is_same_v<F,callback_simple_t<C>>) {
            return  (_obj->*_func)(y);
        }
        return -1;
    }
    virtual z_status callback_mq(MqMsg* msg) {
        if constexpr (std::is_same_v<F,callback_mq_binary_t<C>>)
        {
            if (_binary_size!= msg->data_len)
            {
                return Z_ERROR(zs_data_error);
            }

            return  (_obj->*_func)(msg->data);
        }

        Z_ERROR_LOG("No matching template for http command %s\n",_name.c_str());
        return Z_ERROR(zs_internal_error);
    }


    virtual int callback_http(http_request req,z_string_map &vars,z_json_obj &jin, z_json_stream &jout) {
        if constexpr (std::is_same_v<F,callback_post_json_t<C>>) { return  (_obj->*_func)(jin,jout);   }
        if constexpr (std::is_same_v<F,callback_get_t<C>>) { return  (_obj->*_func)(vars,jout);   }
        if constexpr (std::is_same_v<F,callback_http_delayed_t<C>>) { return  (_obj->*_func)(req,vars,jin,jout);   }
        Z_ERROR_LOG("No matching template for http command %s\n",_name.c_str());
        return -1;
    }
};
//template <class C, typename... Args> using callback_t = z_status (C::*)(Args...);
typedef std::function<void(z_json_stream& json_out)> pending_complete_t;

class DelayedHttpRequest {
public:
    DelayedHttpRequest(http_request r,U64 timeout,pending_complete_t cb) {
        _r=r;
        _ts_expire=timeout+z_time_get_ticks_ms();
        _callback=cb;

    }
    http_request _r;
    U64 _ts_expire;
    std::function<void(z_json_stream& json_out)> _callback;
    void complete() ;
    bool isConnectionId(unsigned long id);
};
template <typename C> class CommandDelayed_t : public Command_t<C,callback_http_delayed_t<C>> {
public:
    using Command::_name;
    CommandDelayed_t(ctext cmd,C *handler,callback_http_delayed_t<C> func) : Command_t<C,callback_http_delayed_t<C>>(cmd,handler,func) {


    }

    virtual ~CommandDelayed_t() override {}

    z_obj_list<MqMsg> _delayed_mq;
    z_obj_list<DelayedHttpRequest,true> _outstanding_reqs;
    std::mutex _mutex_req_list;
    Timer* _req_timer=0;
    int timer_callback_req_wait_expire(void*) {
        delayed_request *dr;
        if (_outstanding_reqs.size()==0)
            return 200; // todo this should be calculated to be the next exipiring
        std::unique_lock mlock(_mutex_req_list);

        U64 now=z_time_get_ticks_ms();

        //WS_DBG("current %d reqs\n",_outstanding_reqs.size());


        _outstanding_reqs.filter_out([now](DelayedHttpRequest *dr) {
            if (dr->_ts_expire > now)
                return false;
            dr->complete();

            return true;
        });
        return 200;// todo this should be calculated to be the next exipiring



    }

    virtual void process_http_close(u_long id) override
    {
        std::unique_lock mlock(_mutex_req_list);

        _outstanding_reqs.filter_out([id](DelayedHttpRequest *dr) {
            if ((id==CONNECTION_CLOSE_ALL) || (dr->isConnectionId(id))) {
                //ZDBG("removing delayed request\n");
                return true;
            }
            return false;
        });

    }

    z_status init ()
    {

        if(!_req_timer) {
            _req_timer=gTimerService.create_timer_t(this,&CommandDelayed_t::timer_callback_req_wait_expire,0 );

        }
        _req_timer->start(200);
        return zs_ok;
    }
    //int process_http_rx(http_request req, cmd_req_type type) override;

    bool add_pending(http_request req,int delay,pending_complete_t callback) {
        std::unique_lock mlock(_mutex_req_list);

        DelayedHttpRequest *dr = new DelayedHttpRequest(req,delay,callback);
        _outstanding_reqs.push_back(dr);

        if(!_req_timer) {
            _req_timer=gTimerService.create_timer_t(this,&CommandDelayed_t::timer_callback_req_wait_expire,0 );

        }
        _req_timer->start(delay);
        return true;

    }
    z_status complete()
    {
        z_string buffer="{ \"nap\": 43 }";
        complete_req_all(buffer);
        return zs_ok;
    }
    z_status complete_req_all()
    {
        DelayedHttpRequest *dr;
        if (_outstanding_reqs.size()==0)
            return zs_ok;
        std::unique_lock mlock(_mutex_req_list);

        ZDBG("completing requests\n");

        // filterout deletes the req
        _outstanding_reqs.filter_out([](DelayedHttpRequest *dr) {
            dr->complete();

            return true;
        });


        return zs_ok;
    }
};


typedef CommandDelayed_t<CommandHandler> CommandDelayed;

class CommandHandler {
public:
    z_obj_map<Command, true> _map;
    //z_obj_list<Command> _owned_commands;
    std::mutex _map_mutex;

    CommandHandler(){}
    virtual ~CommandHandler() {
    }

    int process_http_close(u_long id);


    Command *get_command(ctext name) {
        Command *cmd = nullptr;
        if (!_map.get(name, cmd))
            return nullptr;

        return cmd;
    }
    z_status reg_command(Command* cmd) {
        if (_map.exists(cmd->_name))
            return Z_ERROR(zs_already_exists);
        std::unique_lock mlock(_map_mutex);
        if (!_map.add(cmd->_name, cmd)) {
            return Z_ERROR(zs_already_exists);
        }
        return zs_ok;

    }
    template < typename DATATYPE,typename C>
    Command* reg_bin_func(ctext name,int (C::*func)(DATATYPE*))
    {
        if (_map.exists(name)) {
            Z_ERROR(zs_already_exists);
            return nullptr;
        }
        std::unique_lock mlock(_map_mutex);

        Command *cmd = new Command_t<C,callback_mq_binary_t<C>>(name,(C*)this,(callback_mq_binary_t<C>)func);
        cmd->_binary_size=sizeof(DATATYPE);
        _map.add(name, cmd);
        return cmd;
    }
    template < typename C, typename... Args>
    Command* reg_func(ctext name,int (C::*func)(Args...))
    {
        if (_map.exists(name)) {
            Z_ERROR(zs_already_exists);
            return nullptr;
        }
        std::unique_lock mlock(_map_mutex);
        Command *cmd = new Command_t(name,(C*)this,func);
        _map.add(name, cmd);
        return cmd;
    }
    template < typename C>
    CommandDelayed* reg_func(ctext name,callback_http_delayed_t<C> func)
    {
        if (_map.exists(name)) {
             Z_ERROR(zs_already_exists);
            return nullptr;
        }
        std::unique_lock mlock(_map_mutex);
        CommandDelayed_t<C> *cmd = new CommandDelayed_t(name,(C*)this,func);
        _map.add(name, cmd);
        return (CommandDelayed*)cmd;
    }



};


#endif //ZIPOSOFT_COMMANDHANDLER_H
