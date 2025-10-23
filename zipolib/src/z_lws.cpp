#include "pch.h"
#ifdef WEBSOCKETS
#include "zipolib/z_lws.h"
//#include "webmsg.h"
//#include "z_service_client_lws.h"


ZMETA_DEFV(LwsBase);

ZMETA_DEFV(LwsClient);

template void z_factory_t<LwsClient>::static_add_members_class<LwsClient>(z_factory *f);
/*________________________________________________________________________________________________

LwsWriteQueue

________________________________________________________________________________________________*/

LwsWriteQueue::Writebuff::~Writebuff()
{
	delete _writebuff;
}

LwsWriteQueue::Writebuff::Writebuff(ctext buff, int len)
{
	if (len == -1)
		len = strlen(buff);

	_buff_size = len + LWS_PRE;
	_writebuff = z_new U8[_buff_size];
	
	strncpy((char*)(_writebuff+ LWS_PRE), buff, len);
}
z_status LwsWriteQueue::queue_write(const char* buff)
{
	int len = strlen(buff);
	return queue_write(buff, len);

}
z_status LwsWriteQueue::queue_write(const char* buff, int data_size)
{

	if (_write_queue.get_count() > 20)
		return zs_ok; //TODO!?!
	Writebuff* wb = z_new Writebuff(buff, data_size);
	_write_queue.push(wb);
	return zs_ok;
}
/*
z_string msg;
if (!lws_send_pipe_choked(wsi))
{
if (_msg_queue.pop(msg))
{
n = msg.size();
strncpy((char*)p, msg.c_str(), n);
m = lws_write(wsi, p, n, LWS_WRITE_TEXT);
if (m < n) {
lwsl_err("FUCK %d writing to di socket\n", n);
return -1;
}
}


}
if (_msg_queue.get_count())
{
ZT("queue=%d\n", _msg_queue.get_count());
if (_running)
{
lws_callback_on_writable(wsi);
}

}
*/
int LwsWriteQueue::write_lws(struct lws *wsi)
{
	std::unique_lock<std::mutex> mlock(_mutex);

	Writebuff* wb = 0;
	while (_write_queue.pop(wb))
	{
		if (lws_send_pipe_choked(wsi))
		{
			lws_callback_on_writable(wsi);
			return 0;
		}
			
		int sent = lws_write(wsi, wb->get_write_ptr(), wb->get_write_len(), LWS_WRITE_TEXT);
		
		if (sent < (int)wb->get_write_len())
		{
			delete wb;
			return -1; //error
		}
		delete wb;
	}

	return 0;
}
int LwsWriteQueue::write_lwsbroadcast(struct lws *wsi)
{
	std::unique_lock<std::mutex> mlock(_mutex);

	Writebuff* wb = 0;

	if (_write_queue.front(wb))
	{

		if (lws_send_pipe_choked(wsi))
		{
			
			return -1;
		}

		int sent = lws_write(wsi, wb->get_write_ptr(), wb->get_write_len(), LWS_WRITE_TEXT);
		if (sent < (int)wb->get_write_len())
			return -1; //error

	}


	return 0;
}
void LwsWriteQueue::empty_queue()
{
	Writebuff* wb = 0;
	while (_write_queue.pop(wb))
	{
		delete wb;
	}


}

LwsWriteQueue:: ~LwsWriteQueue()
{
	empty_queue();

}


/*________________________________________________________________________________________________

LwsClientBase

________________________________________________________________________________________________*/
z_status LwsClientBase::write_socket(const char* buff, int data_size)
{
	std::unique_lock<std::mutex> mlock(_mutex);

	
	if (!_connected)
		return zs_not_open;
	if (!_wsi)
	{
		Z_ERROR_MSG(zs_internal_error, "wsi==0!!!!!!");
		return zs_internal_error;

	}
	queue_write(buff, data_size);
	int result = lws_callback_on_writable(_wsi);


	return zs_ok;
}


void LwsClientBase::callback_rx(ctext in, int len)
{
	gz_stdout << in << "\n";
}
int LwsClientBase::callback_write(struct lws *wsi)
{
	if (!_connected)
		return -1;
	return write_lws(wsi);
}

/*________________________________________________________________________________________________

LwsServerClient

________________________________________________________________________________________________*/

/*________________________________________________________________________________________________

LwsBase


________________________________________________________________________________________________*/
z_status LwsBase::broadcast_write_old(const char* buff, int data_size)
{
	if (!_running)
		return zs_not_open;

	std::unique_lock<std::mutex> mlock(_mutex);

	empty_queue();
	queue_write(buff, data_size);

	start_write_all(0);


	return zs_ok;
}
z_status LwsBase::broadcast_write(const char* buff, int data_size)
{
	if (!_running)
		return zs_not_open;

	std::unique_lock<std::mutex> mlock(_mutex);
	for (auto i : _clients)
	{
		
		i->write_socket(buff, data_size);

	}


	return zs_ok;
}




enum demo_protocols {
	PROTOCOL_HTTP = 0,
	PROTOCOL_CONTROLLER,
	PROTOCOL_CONTROLLER_TXT,
	PROTOCOL_BRODCAST,
	/* always last */
	DEMO_PROTOCOL_COUNT
};




#define LWSCB(_X_) \
static int _X_(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len){\
	LwsBase* prv = (LwsBase*)lws_context_user(lws_get_context(wsi));\
	return prv->_X_(wsi, reason, user, in, len);}

LWSCB(callback_controller);
LWSCB(callback_brodcast);
LWSCB(callback_http);



static struct lws_protocols protocols[] = {
	/* rx buf size must be >= permessage-deflate rx size */

	{
		"http-only",callback_http,	sizeof(LwsBase::UserContext),512, PROTOCOL_HTTP, (void*)LwsBase::t_controller_web /* proto user ptr*/
	},
	{
		"controller",callback_controller,	sizeof(LwsBase::UserContext),512, PROTOCOL_CONTROLLER, (void*)LwsBase::t_controller_web /* proto user ptr*/
	},
	{  //this is for a command line connect
		"controller_txt",callback_controller,	sizeof(LwsBase::UserContext),512, PROTOCOL_CONTROLLER_TXT,  (void*)LwsBase::t_controller_txt  /* proto user ptr*/
	},
	{
		"broadcast",callback_brodcast,	sizeof(LwsBase::UserContext),512, PROTOCOL_BRODCAST, 0 /* proto user ptr*/
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};

int LwsBase::callback_controller(struct lws *wsi, enum lws_callback_reasons reason,
	void *user, void *in, size_t len)
{

	UserContext *context = (UserContext *)user;
	LwsServerClient* client = 0;
	if(context)
		client=context->client;

	switch (reason) {

	case LWS_CALLBACK_CLOSED:
		LWST("LWS_CALLBACK_CLOSED wsi=%x", wsi);
		//context->object->_wsi = 0;
		break;


	case LWS_CALLBACK_ESTABLISHED:
	{
		ControllerType controller_type = (ControllerType)(size_t)lws_get_protocol(wsi)->user;
		// this will create the type of client
		// right now eight a text or json factory controller client
		callback_controller_client_connect(wsi, context, controller_type);
		//context->object = z_new z_service_client_lws(wsi, this);
		//lws_callback_on_writable(wsi);
		context->id = _client_id++;
		LWST("LWS_CALLBACK_ESTABLISHED %d, wsi=%x", context->id, wsi);
	}
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
	{
		LWST("LWS_CALLBACK_SERVER_WRITEABLE %d, wsi=%x", context->id, wsi);
		if(client)
			return client->callback_write(wsi);
		return 0;
	}
	break;

	case LWS_CALLBACK_RECEIVE:
		LWST("LWS_CALLBACK_RECEIVE %d", context->id);
		if (client)
			client->callback_rx((ctext)in, len);
		return 0;
		break;


	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		ZT("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE=%x", wsi);
		if (client)
			client->close();
		//_saved_wsi
		break;
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		ZT("LWS_CALLBACK_CLIENT_ESTABLISHED wsi=%x", wsi);
		break;
	case LWS_CALLBACK_CLIENT_WRITEABLE:
		ZT("LWS_CALLBACK_CLIENT_WRITEABLE wsi=%x", wsi);
		break;
	default:
		break;
	}

	return 0;
}


int LwsBase::callback_brodcast(struct lws *wsi, enum lws_callback_reasons reason,
	void *user, void *in, size_t len)
{
	//debug_protocol(reason);

	UserContext *context = (UserContext *)user;
	switch (reason) {
	case LWS_CALLBACK_CLOSED:
		ZT("LWS_CALLBACK_CLOSED wsi=%x", wsi);
		//context->object->_wsi = 0;
		break;


	case LWS_CALLBACK_ESTABLISHED:
		//lws_callback_on_writable(wsi);

		context->id = _client_id++;
		ZT("LWS_CALLBACK_ESTABLISHED %d, wsi=%x", context->id, wsi);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
	{
		LWST("LWS_CALLBACK_SERVER_WRITEABLE %d, wsi=%x", context->id, wsi);
		int sent = write_lwsbroadcast(wsi);
		if (sent == -1)
			start_write_all(0);
		break;

	}
	break;




	default:
		break;
	}

	return 0;
}




LwsBase::LwsBase(int port)
{
	_running = false;
	_debug_level = 3;
	_port = port;
	_poll_ms = 150;
}
LwsBase::~LwsBase()
{
	stop();
}

z_status LwsBase::testmsg()
{

	if (!_running)
		return zs_not_open;

	queue_write("fuck me");
	//send_line("wtf");
	return zs_ok;
}


void LwsBase::start_write_all(int protocol)
{
	lws_callback_on_writable_all_protocol(_lws_context,
		&protocols[PROTOCOL_BRODCAST]);
}

void LwsBase::custom_emit_syslog(int level, const char *line)
{

	get_zlog().get_stream_dbg().write_str(line);
}
z_status LwsBase::autostart()
{

	if (_auto_start)
		return start();
	return zs_ok;

}
z_status LwsBase::start()
{

	if (_running)
		return zs_already_open;

	z_status s = init();
	if (s)
		return s;
	_running = true;
	
	_h_thread = std::thread(&LwsBase::thread, this);

	return zs_ok;

}
void LwsBase::server_callback_rx(ctext in, int len)
{
	gz_stdout << in << "\n";
}
z_status LwsBase::stop()
{
	if (!_running)
		return zs_ok;

	_running = false;
	lws_cancel_service(_lws_context);
	if (_h_thread.joinable())
		_h_thread.join();
	ZT("stopped");

	lws_context_destroy(_lws_context);

	_lws_context = 0;
	return zs_ok;

}



z_status LwsBase::init()
{
	struct lws_context_creation_info info;

	memset(&info, 0, sizeof info);
	info.port = _port;

	/* tell the library what debug level to emit and to send it to syslog */
	lws_set_log_level(_debug_level, custom_emit_syslog);

	info.iface = NULL;
	info.protocols = protocols;
	info.ssl_cert_filepath = NULL;
	info.ssl_private_key_filepath = NULL;
	info.user = this;
	info.gid = -1;
	info.uid = -1;
	info.max_http_header_pool = 16;
	info.options = LWS_SERVER_OPTION_VALIDATE_UTF8;
	info.extensions = NULL;// exts;
	info.timeout_secs = 5;
	/*
	info.ssl_cipher_list = "ECDHE-ECDSA-AES256-GCM-SHA384:"
		"ECDHE-RSA-AES256-GCM-SHA384:"
		"DHE-RSA-AES256-GCM-SHA384:"
		"ECDHE-RSA-AES256-SHA384:"
		"HIGH:!aNULL:!eNULL:!EXPORT:"
		"!DES:!MD5:!PSK:!RC4:!HMAC_SHA1:"
		"!SHA1:!DHE-RSA-AES128-GCM-SHA256:"
		"!DHE-RSA-AES128-SHA256:"
		"!AES128-GCM-SHA256:"
		"!AES128-SHA256:"
		"!DHE-RSA-AES256-SHA256:"
		"!AES256-GCM-SHA384:"
		"!AES256-SHA256";
		*/


	_lws_context = lws_create_context(&info);
	if (_lws_context == NULL) {
		return Z_ERROR(zs_internal_error);
	}
	return zs_ok;
}

int LwsBase::thread()
{
	int n = 0;
	while (n >= 0 && _running) {
		n = lws_service(_lws_context, _poll_ms);

	}
	return 0;
}


int LwsBase::debug_protocol( enum lws_callback_reasons reason)
{
	switch (reason) {
		//ignore:
	case LWS_CALLBACK_LOCK_POLL:
	case LWS_CALLBACK_UNLOCK_POLL:
	case LWS_CALLBACK_GET_THREAD_ID:
	case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
	case LWS_CALLBACK_ADD_POLL_FD:
	case LWS_CALLBACK_WSI_CREATE:

		break;

	case LWS_CALLBACK_CLOSED:
		ZT("LWS_CALLBACK_CLOSED");
		break;

	case LWS_CALLBACK_PROTOCOL_INIT:
		ZT("LWS_CALLBACK_PROTOCOL_INIT");
		break;
	case LWS_CALLBACK_PROTOCOL_DESTROY:
		ZT("LWS_CALLBACK_PROTOCOL_DESTROY");
		break;
	case LWS_CALLBACK_ESTABLISHED:
		ZT("LWS_CALLBACK_ESTABLISHED ");
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		ZT("LWS_CALLBACK_SERVER_WRITEABLE");
	break;

	case LWS_CALLBACK_RECEIVE:
		ZT("LWS_CALLBACK_RECEIVE");

		break;
	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		ZT("LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION");
		break;
	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		ZT("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE");
		break;

	default:
		ZT("REASON=%d", reason);
		break;
	}

	return 0;
}


int LwsBase::callback_debug_protocol(struct lws *wsi, enum lws_callback_reasons reason,
	void *user, void *in, size_t len)
{
	int n;
	UserContext *context = (UserContext *)user;
	switch (reason) {
	case LWS_CALLBACK_CLOSED:
		LWST("LWS_CALLBACK_CLOSED");
		break;
	case LWS_CALLBACK_LOCK_POLL:
		/*
		* lock mutex to protect pollfd state
		* called before any other POLL related callback
		* if protecting wsi lifecycle change, len == 1
		*/
		break;

	case LWS_CALLBACK_UNLOCK_POLL:
		/*
		* unlock mutex to protect pollfd state when
		* called after any other POLL related callback
		* if protecting wsi lifecycle change, len == 1
		*/
		break;
	case LWS_CALLBACK_PROTOCOL_INIT:
		LWST("LWS_CALLBACK_PROTOCOL_INIT");
		break;
	case LWS_CALLBACK_PROTOCOL_DESTROY:
		LWST("LWS_CALLBACK_PROTOCOL_DESTROY");
		break;
	case LWS_CALLBACK_ESTABLISHED:
		LWST("LWS_CALLBACK_ESTABLISHED %d", context->id);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
	{
		LWST("LWS_CALLBACK_SERVER_WRITEABLE");

#if 0
		z_string msg;
		if (!lws_send_pipe_choked(wsi))
		{
			if (_msg_queue.pop(msg))
			{
				n = msg.size();
				strncpy((char*)p, msg.c_str(), n);
				m = lws_write(wsi, p, n, LWS_WRITE_TEXT);
				if (m < n) {
					lwsl_err("FUCK %d writing to di socket\n", n);
					return -1;
				}
			}


		}
		if (_msg_queue.get_count())
		{
			ZT("queue=%d\n", _msg_queue.get_count());
			if (_running)
			{
				lws_callback_on_writable(wsi);
			}

		}

#endif
	}
	break;

	case LWS_CALLBACK_RECEIVE:
		LWST("LWS_CALLBACK_RECEIVE %d", context->id);

		break;
		/*
		* this just demonstrates how to use the protocol filter. If you won't
		* study and reject connections based on header content, you don't need
		* to handle this callback
		*/
	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		ZT("LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION");
		//dump_handshake_info(wsi);
		/* you could return non-zero here and kill the connection */
		break;

		/*
		* this just demonstrates how to handle
		* LWS_CALLBACK_WS_PEER_INITIATED_CLOSE and extract the peer's close
		* code and auxiliary data.  You can just not handle it if you don't
		* have a use for this.
		*/
	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		ZT("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE");
		lwsl_notice("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: len %d\n",
			len);
		for (n = 0; n < (int)len; n++)
			lwsl_notice(" %d: 0x%02X\n", n,
			((unsigned char *)in)[n]);
		break;

	default:
		break;
	}

	return 0;
}
/*________________________________________________________________________________________________

LwsServer


________________________________________________________________________________________________*/
LwsServer::LwsServer(int port) : LwsBase(port)
{
}
LwsServer::~LwsServer()
{
}

/*________________________________________________________________________________________________

LwsClient


________________________________________________________________________________________________*/

z_status LwsClient::connect(ctext address, int port)
{
	if (is_running())
		return zs_already_open;
	_address = address;
	_port = port;
	return start(); //calls init below

}


z_status LwsClient::init()
{
	struct lws_context_creation_info info;

	memset(&info, 0, sizeof info);

	/* tell the library what debug level to emit and to send it to syslog */
	lws_set_log_level(_debug_level, custom_emit_syslog);
	info.port = CONTEXT_PORT_NO_LISTEN;

	info.iface = NULL;
	info.protocols = protocols;
	info.ssl_cert_filepath = NULL;
	info.ssl_private_key_filepath = NULL;
	info.user = this;
	info.gid = -1;
	info.uid = -1;
	info.max_http_header_pool = 16;
	info.options = LWS_SERVER_OPTION_VALIDATE_UTF8;
	info.extensions = NULL;// exts;
	info.timeout_secs = 5;

	_lws_context = lws_create_context(&info);
	if (_lws_context == NULL) {
		return Z_ERROR(zs_internal_error);
	}
	/**
	* struct lws_client_connect_info - parameters to connect with when using
	*				    lws_client_connect_via_info()
	*
	* @context:	lws context to create connection in
	* @address:	remote address to connect to
	* @port:	remote port to connect to
	* @ssl_connection: nonzero for ssl
	* @path:	uri path
	* @host:	content of host header
	* @origin:	content of origin header
	* @protocol:	list of ws protocols
	* @ietf_version_or_minus_one: currently leave at 0 or -1
	* @userdata:	if non-NULL, use this as wsi user_data instead of malloc it
	* @client_exts: array of extensions that may be used on connection
	* @method:	if non-NULL, do this http method instead of ws[s] upgrade.
	*		use "GET" to be a simple http client connection
	* @parent_wsi:	if another wsi is responsible for this connection, give it here.
	*		this is used to make sure if the parent closes so do any
	*		child connections first.
	* @uri_replace_from: if non-NULL, when this string is found in URIs in
	*		text/html content-encoding, it's replaced with @uri_replace_to
	* @uri_replace_to: see above
	* @vhost:	vhost to bind to (used to determine related SSL_CTX)
	*/
	memset(&_client_info, 0, sizeof(_client_info));
	_client_info.context = _lws_context;
	_client_info.ssl_connection = NULL;
	_client_info.address = _address.c_str();
	_client_info.path = _address.c_str();
	_client_info.port = _port;
	_client_info.host = _client_info.address;
	_client_info.origin = _client_info.address;
	_client_info.ietf_version_or_minus_one = -1;
	_client_info.client_exts = NULL;
	_client_info.protocol = _protocol;
	_wsi = 0;
	return zs_ok;
}
int LwsClient::thread()
{
	int n = 0;
	while (n >= 0 && _running) {
		if (!_wsi)
		{
			ZT("trying to connect");
			_wsi = lws_client_connect_via_info(&_client_info);
			if (_wsi)
			{
				ZT("connected");
			}
		}
		n = lws_service(_lws_context, _poll_ms);

	}
	ZT("exiting client thread");
	return 0;
}


int LwsClient::callback_controller(struct lws *wsi, enum lws_callback_reasons reason,
	void *user, void *in, size_t len)
{
	debug_protocol(reason);
	UserContext *context = (UserContext *)user;
	switch (reason) {
		
	case LWS_CALLBACK_CLOSED:
		ZT("LWS_CALLBACK_CLOSED wsi=%x", wsi);
		_connected = false;

		//context->object->_wsi = 0;
		break;

	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		ZT("LWS_CALLBACK_CLIENT_CONNECTION_ERROR wsi=%x", wsi);
		break;
	case LWS_CALLBACK_ESTABLISHED:
		ZT("LWS_CALLBACK_ESTABLISHED wsi=%x", wsi);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		ZT("LWS_CALLBACK_SERVER_WRITEABLE wsi=%x", wsi);
		break;

	case LWS_CALLBACK_RECEIVE:
		ZT("LWS_CALLBACK_RECEIVE %d", context->id);

		break;


		/*
		* this just demonstrates how to handle
		* LWS_CALLBACK_WS_PEER_INITIATED_CLOSE and extract the peer's close
		* code and auxiliary data.  You can just not handle it if you don't
		* have a use for this.
		*/
	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		ZT("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE=%x", wsi);
		break;
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		ZT("LWS_CALLBACK_CLIENT_ESTABLISHED wsi=%x", wsi);
		_connected = true;
		break;
	case LWS_CALLBACK_CLIENT_RECEIVE:
		ZT("LWS_CALLBACK_CLIENT_RECEIVE wsi=%x", wsi);
		callback_rx((ctext)in, len);

		break;
	case LWS_CALLBACK_CLIENT_WRITEABLE:
		ZT("LWS_CALLBACK_CLIENT_WRITEABLE wsi=%x", wsi);
		callback_write(wsi);

		break;
	default:
		break;
	}

	return 0;
}
#endif