//
// Created by ac on 12/15/24.
//
#include "WebRequests.h"
#include "parson/parson.h"

#include "WebServer.h"

#include "JsonCmd.h"
#include "root.h"

#if 1
#define WS_DBG(...)
#else
#define WS_DBG ZDBG

#endif
int callback_rx(ctext in, int len,
                ctext *p_status_msg // TODO, bad bad bad
);

void get_var_map(mg_str buf,z_string_map& var_map) {
    mg_str entry, k, v;
    while (mg_span(buf, &entry, &buf, '&')) {
        if (mg_span(entry, &k, &v, '=')
            ) {
            z_string val;
            val.assign(v.ptr,v.len);
            z_string key;
            key.assign(k.ptr,k.len);
            var_map[key] = val;


            }
    }
}


void complete_delayed_req(delayed_request *dr) {
    z_string s;
    z_json_stream js(s);
    //RfidReader &reader = root.getReader();
    js.obj_start();
    js.keyval_int("ts",z_time::get_now_ms());

    (*(dr->fn_complete))(js,dr->ctx1,dr->ctx2);
    js.obj_end();

    //reader.get_reads_since(js, req->index);
    //z_string msg;
    //msg.format("{\"conn_id\":%d ,\"count\":%d}"            , req->conn_id, _req_count);
    mg_wakeup(dr->r.c->mgr, dr->r.c->id, s.c_str(), s.length()); // Respond to parent
    WS_DBG("wakeup: %s\n ", s.c_str());
    // free((void *) dr->r.hm->message.ptr); // Free all resources that were


}

z_status WebServer::stop() {
    if (!_running)
        return zs_ok;
    _req_timer->stop();

    _running = false;
    if (_h_thread.joinable())
        _h_thread.join();
    WS_DBG("stopped");


    return zs_ok;
}

z_status WebServer::complete_by_id(unsigned long id) {
    delayed_request *dr;
    if (_outstanding_reqs.size()==0)
        return zs_ok;
    //std::unique_lock<std::mutex> mlock(_mutex);


    WS_DBG("completing %d reqs\n",_outstanding_reqs.size());

    _outstanding_reqs.filter_out([id](delayed_request *dr) {
        if (dr->r.c->id == id) {
            complete_delayed_req( dr);
            return true;
        }
        return false;

    });


    return zs_ok;

}

z_status WebServer::complete_all()
{
    delayed_request *dr;
    if (_outstanding_reqs.size()==0)
        return zs_ok;
    std::unique_lock<std::mutex> mlock(_mutex);


    WS_DBG("completing %d reqs\n",_outstanding_reqs.size());

    _outstanding_reqs.filter_out([](delayed_request *dr) {
        complete_delayed_req( dr);

        return true;
    });


    return zs_ok;
}

ctext HEADERS="HTTP/1.1 %d OK\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: Origin, Content-Type, X-Auth-Token\r\nAccess-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
              "Content-Type: application/json; charset=utf-8\r\n"
              "Transfer-Encoding: chunked\r\n\r\n";
void send_headers(struct mg_connection *c,int status) {
    mg_printf(c, HEADERS,status);
}






static int process_command(http_request req,cmd_req_type type) {

    size_t i;
    for (i=0;i<cmd_list_size;i++)
    {
        auto cmd = cmd_list[i];
        if (mg_http_match_uri(req.hm, cmd.cmd_name) && (type==cmd.type)) {
            // Single-threaded code path, for performance comparison
            // The /fast URI responds immediately
            if (type==REQUEST_GET)
            {
                z_string_map var_map;
                get_var_map(req.hm->query,var_map);
                fn_cmd_get_t fn=  (fn_cmd_get_t)  cmd_list[i].fn;
                return  (*fn)(req,var_map);
            }
            if (type==REQUEST_POST)
            {
                zp_text_parser p;
                z_json_obj obj=p.parseJsonObj(req.hm->body.ptr,req.hm->body.len);
                fn_cmd_post_t fn=(fn_cmd_post_t)cmd_list[i].fn;
                (*fn)(req,obj);
                return 0;
            }
            break;

        }
    }
    mg_http_reply(req.c, 404, "Access-Control-Allow-Origin: *\r\nContent-Type:text/plain\r\nAccess-Control-Allow-Headers: Origin, Content-Type, X-Auth-Token\r\nAccess-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n", "REQ not found\n");

    return 0;
}


void WebServer::event_handler(struct mg_connection *c, int ev, void *ev_data) {
    std::unique_lock<std::mutex> mlock(_mutex);
    if (ev == MG_EV_POLL) {
        return;
    }
    if (ev == MG_EV_READ) {
        return;
    }
    if (ev == MG_EV_WRITE) {
        return;
    }
    if (ev == MG_EV_HTTP_HDRS) {
        return;
    }

    if (ev == MG_EV_CLOSE) {
        WS_DBG("[%d] MG_EV_CLOSE\n",c->id);
        complete_by_id(c->id);
        return;
    }
    if (ev == MG_EV_OPEN) {
        WS_DBG("[%d] MG_EV_OPEN\n",c->id);
        return;
    }
    if (ev == MG_EV_ACCEPT) {
        WS_DBG("[%d] MG_EV_ACCEPT\n",c->id);
        return;
    }
    // this is a delayed request being completed
    if (ev == MG_EV_WAKEUP) {
        WS_DBG("[%d] MG_EV_WAKEUP\n",c->id);

        struct mg_str *data = (struct mg_str *) ev_data;
        mg_http_reply(c, 200, "Content-Type:application/json\r\nAccess-Control-Allow-Origin: *\r\nAccess-Control-Allow-Headers: Origin, Content-Type, X-Auth-Token\r\nAccess-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n", "%.*s\n",
            data->len, data->ptr);
    }
    if (ev == MG_EV_HTTP_MSG) {
        _req_count++;

        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        WS_DBG("[%d] MG_EV_HTTP_MSG: %*.s\n",c->id,hm->uri.len,hm->uri.ptr);



        if (mg_http_match_uri(hm, "/")) {
            // Print some statistics about currently established connections
            mg_printf(c, "HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n");
            mg_http_printf_chunk(c, "ID PROTO TYPE      LOCAL           REMOTE\n");
            for (struct mg_connection *t = c->mgr->conns; t != NULL; t = t->next) {
                mg_http_printf_chunk(c, "%-3lu %4s %s %M %M\n", t->id,
                                     t->is_udp ? "UDP" : "TCP",
                                     t->is_listening
                                         ? "LISTENING"
                                         : t->is_accepted
                                               ? "ACCEPTED "
                                               : "CONNECTED",
                                     mg_print_ip, &t->loc, mg_print_ip, &t->rem);
            }
            mg_http_printf_chunk(c, ""); // Don't forget the last empty chunk
            return;
        }
        cmd_req_type type=REQUEST_INVALID;
        if (hm->method.len == 4 && !memcmp(hm->method.ptr, "POST", 4)) {
            // Verify it's a POST request
            // Extract POST data from hm->body
            // Example: print the received body

            WS_DBG("Received POST data: %.*s", (int)hm->body.len, hm->body.ptr);
            type=REQUEST_POST;

        }
        if (hm->method.len == 3 && !memcmp(hm->method.ptr, "GET", 3)) {

            type=REQUEST_GET;
        }
        process_command({c,hm,_req_count}, type);
        return;
    }
    WS_DBG("[%d] ev=%d\n",c->id, ev);
}

z_status WebServer::connect(ctext address, int port) {
    if (is_running())
        return zs_already_open;
    _address = address;
    _port = port;
    return start();
}
int WebServer::timer_callback_req_wait_expire(void*) {
    delayed_request *dr;
    if (_outstanding_reqs.size()==0)
        return 200;
    std::unique_lock<std::mutex> mlock(_mutex);

    U64 now=z_time_get_ticks();

    //WS_DBG("current %d reqs\n",_outstanding_reqs.size());


    _outstanding_reqs.filter_out([now](delayed_request *dr) {
        if (dr->ts_expire > now)
            return false;
        //ZDBG("completeing req [%lld] now=%lld\n",dr->ts_expire,now);
        complete_delayed_req( dr);
        return true;
    });
    return 200;



}

z_status WebServer::start() {
    if (is_running())
        return zs_already_open;

    _running = true;
    _h_thread = std::thread(&WebServer::thread, this);
    if(!_req_timer) {
        _req_timer=root.timerService.create_timer_t(this,&WebServer::timer_callback_req_wait_expire,0 );

    }
    root.getReader().register_consumer(this);

    _req_timer->start(200);
    return zs_ok;
}

z_status WebServer::set_log_level(int ll) {
    _log_level = ll;
    mg_log_set(_log_level); // Set log level

    return zs_ok;
}
bool WebServer::callbackQueueEmpty()
{
    complete_all();

    return true;
}

bool WebServer::callbackRead(RfidRead* read)
{
    return true;
}
//static const char *s_http_addr = "http://0.0.0.0:8000";    // HTTP port
char s_http_addr[40];
static void http_callback(struct mg_connection *c, int ev, void *ev_data) {
    WEBSERV(c).event_handler(c, ev, ev_data);
}

int WebServer::thread() {
    struct mg_mgr mgr; // Event manager

    mg_log_set(_log_level); // Set log level
    mg_mgr_init(&mgr); // Initialise event manager
    sprintf(s_http_addr, "http://0.0.0.0:%d", _port);
    printf("connecting to: %s\n", s_http_addr);
    mg_http_listen(&mgr, s_http_addr, http_callback, this); // Create HTTP listener
    mg_wakeup_init(&mgr); // Initialise wakeup socket pair

    while (_running) {
        mg_mgr_poll(&mgr, 100);
    }
    mg_mgr_free(&mgr);

    return 0;
}

ZMETA_DEF(WebServer);
