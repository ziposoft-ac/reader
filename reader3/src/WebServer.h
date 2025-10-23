//
// Created by ac on 12/15/24.
//

#ifndef WEBSERVER_H
#define WEBSERVER_H
#include "pch.h"
#include "timers.h"

typedef struct delayed_request delayed_request;
typedef struct mg_connection mg_connection;
typedef struct http_request http_request;

class WebServer {
    std::thread _h_thread;
    virtual int thread();
    bool _running=false;
    std::mutex _mutex;
    Timer* _req_timer=0;
    int timer_callback_req_wait_expire(void*);

public:
    int _log_level=8000;
    int _port;
    int _req_count=0;
    z_string _address;
    z_obj_list<delayed_request> _outstanding_reqs;


    WebServer(){}
    virtual ~WebServer() {}
    z_status stop();
    z_status set_log_level(int ll);
    z_status start();
    z_status complete_all();
    z_status complete_by_id(unsigned long id);
    virtual z_status connect(ctext address, int port);
    bool is_running() {
        return _running;
    }
    void event_handler(struct mg_connection *c, int ev, void *ev_data);
    int get_reads(struct mg_connection *c, struct mg_http_message *hm);
};

#define WEBSERV(c) (*(WebServer *) ((c)->fn_data))

ZMETA_DECL(WebServer) {

    ZSTAT(is_running);
    ZPROP(_log_level);
    ZPROP(_port);
    ZPROP(_address);
    ZACT(stop);
    ZACT(start);
    ZACT(complete_all);

    ZCMD(set_log_level, ZFF_CMD_DEF, "set_log_level",
         ZPRM(int, ll, 0, "ll", ZFF_PARAM)
    );

}




#endif //WEBSERVER_H
