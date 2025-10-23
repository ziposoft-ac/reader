#ifndef z_lws_h
#define z_lws_h

#include "zipolib/zipolib.h"
#include "zipolib/z_safe_queue.h"

// MUST INCLUDE THIS FOR FACTORY MACROS TO WORK!
#include "zipolib/z_factory_controller.h"

#include <libwebsockets.h>
#define WRITE_BUFF_SIZE 3000

#undef  DEBUG_LWS 

#if DEBUG_LWS
#define LWST ZT
#else
#define LWST(...)
#endif


class WebMsg
{
public:
	virtual ctext type() = 0;
	virtual void get_json(z_string &json);
	virtual ~WebMsg() {}
};
class LwsWriteQueue
{
	class Writebuff
	{
		U8* _writebuff = 0;
		int _buff_size = 0;
	public:
		Writebuff(ctext data, int len);
		~Writebuff();

		U8* get_write_ptr() {
			return _writebuff + LWS_PRE;
		}
		size_t  get_write_len()
		{
			return _buff_size - LWS_PRE;
		}

	};
	z_safe_queue<Writebuff*> _write_queue;



	//Writebuff _broadcast_write;
	//unsigned char _write_buf[LWS_PRE + WRITE_BUFF_SIZE];
	//unsigned char* _write_ptr;
	//int _write_size = 0;
protected:
	std::mutex _mutex;

public:
	LwsWriteQueue() {}
	virtual ~LwsWriteQueue();
	z_status queue_write(const char* buff, int data_size);
	z_status queue_write(const char* buff);
	int  write_lws(struct lws *wsi);

	void empty_queue();

	// BUG FIX!!!
	//z_status queue_write_broadcast(const char* buff, int data_size);
	int  write_lwsbroadcast(struct lws *wsi);
};
/*______________________________________________

LwsClientBase
______________________________________________*/
class LwsClientBase : public LwsWriteQueue
{
protected:
	bool _connected = false;
	struct lws *_wsi = 0;

public:

	LwsClientBase()
	{
	}
	virtual ~LwsClientBase()
	{

	}
	virtual z_status write_socket(const char* buff, int data_size = -1);

	int callback_write(struct lws *wsi);
	virtual void callback_rx(ctext in, int len);

	void close()
	{
		std::unique_lock<std::mutex> mlock(_mutex);
		_connected = false;
		_wsi = 0;
	}


};
class LwsBase;
/*______________________________________________

LwsServerClient
______________________________________________*/
class LwsServerClient : public LwsClientBase
{

public:
	LwsBase* _server = 0;
	LwsServerClient(struct lws * wsi, LwsBase* server)
	{
		_server = server;
		_wsi = wsi;
		_connected = true;
	}
	virtual ~LwsServerClient()
	{

	}
};
/*______________________________________________

LwsBase
______________________________________________*/
class LwsBase : public LwsWriteQueue
{



	int _client_id = 0;

	void start_write_all(int protocol);
	std::thread _h_thread;



protected:
	struct lws_context *_lws_context;
	virtual int thread();
	bool _running=false;
	virtual z_status init();
	z_string _buff;

	LwsServerClient* _pBroadCast = 0;
	z_obj_vector<LwsServerClient> _clients;


public:

	enum ControllerType {
		t_controller_web,
		t_controller_txt,
	}; //ug this is terrible

	struct UserContext
	{
		int id = 0;
		LwsServerClient* client = 0;
	};

	static void custom_emit_syslog(int level, const char *line);


	z_status broadcast_write_old(const char* buff, int data_size = -1);
	z_status broadcast_write(const char* buff, int data_size = -1);

	//props
	int _port;
	int _poll_ms;
	int _debug_level;
	bool _auto_start = true;


	LwsBase(int port);
	virtual ~LwsBase();

	bool is_running() {
		return _running;
	}

	virtual void server_callback_rx(ctext in, int len);

	z_status autostart();
	z_status start();
	z_status stop();
	z_status testmsg();

	virtual int callback_controller_client_connect(struct lws *wsi, UserContext* context, ControllerType type)
	{
		return 0;
	}

	int callback_brodcast(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
	virtual int callback_controller(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

	int callback_debug_protocol(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
	int debug_protocol( enum lws_callback_reasons reason);

	virtual int server_callback_write(struct lws *wsi)
	{
		return write_lws(wsi);
	}

	//HTTP stuff
	int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

	int json_request(ctext req, lws *wsi,z_strlist& args);
	virtual int callback_http_request(ctext req,z_strlist& args, z_string & content_type, z_string & output);

};
/*______________________________________________

LwsServer
______________________________________________*/
class LwsServer : public LwsBase
{
public:
	LwsServer(int port);
	virtual ~LwsServer();
};
/*______________________________________________

LwsClient
______________________________________________*/
class LwsClient : public LwsBase,public LwsClientBase
{
	friend z_factory_t<LwsClient>;
	z_string _address;
	z_string _protocol = "controller_txt";
	struct lws_client_connect_info _client_info;
public:
	LwsClient() : LwsBase(-1)
	{
	}
	virtual ~LwsClient() {};
	virtual z_status init();
	virtual int thread();
	virtual int callback_controller(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
	virtual z_status connect(ctext address, int port);
	virtual z_status writeout(const char* buff)
	{
		return write_socket(buff);
	}

	bool is_connected() {
		return _connected;
	}
};

ZMETA_DECL(WebMsg)
{
	ZSTAT(type);
}
ZMETA_DECL(LwsBase)
{
	ZPROP(_port);
	ZPROP(_auto_start);
	ZPROP(_poll_ms);
	ZPROP(_debug_level);
	ZACT(start);
	ZACT(stop);
	ZACT(autostart);
	ZACT(testmsg);
	ZSTAT(is_running);

	
	//ZCMD(send_line, ZFF_CMD_DEF, "send_line",ZPRM(z_string, msg, "dead", "msg", ZFF_PARAM));


};

ZMETA_DECL(LwsClient)
{
	ZPROP(_address);
	ZPROP(_protocol);
	ZSTAT(is_connected);

	ZBASE(LwsBase);
	ZCMD(connect, ZFF_CMD_DEF, "connect to server",
		ZPRM(z_string, address, "localhost", "address of server", ZFF_PARAM),
		ZPRM(int, port, 7777, "port of server", ZFF_PARAM)
	);
	ZCMD(writeout, ZFF_CMD_DEF, "write to server",
		ZPRM(z_string, msg, "bla bla", "message to wrtie", ZFF_PARAM)
	);
};



#endif