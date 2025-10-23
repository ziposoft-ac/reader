#include "pch.h"
#ifdef WEBSOCKETS

#include "zipolib/z_lws.h"




void
dump_handshake_info(struct lws *wsi)
{
	ZTF;
	int n = 0, len;
	char buf[256];
	const unsigned char *c;
	do {
		c = lws_token_to_string((lws_token_indexes)n);
		if (!c) {
			n++;
			continue;
		}

		len = lws_hdr_total_length(wsi, (lws_token_indexes)n);
		if (!len || len > sizeof(buf) - 1) {
			n++;
			continue;
		}

		lws_hdr_copy(wsi, buf, sizeof buf, (lws_token_indexes)n);
		buf[sizeof(buf) - 1] = '\0';
		ZDBG("    %s = %s\n", (char *)c, buf);
		n++;
	} while (c);
}

int LwsBase::json_request(ctext req, lws *wsi, z_strlist& args)
{
	//ZTF;
	U8 buffer[4096];
	U8 *end, *start;
	int n;
	U8 *p;
	p = buffer;
	start = p;
	end = p + sizeof(buffer) - LWS_PRE;

	z_string msg;
	z_string content_type = "application/json";
	z_string attachment;

	if (strstr(req, "csv"))
	{
		// TODO - FIX HACK
		content_type = "text/csv";
		attachment = "attachment; filename = \"results.csv\"";

	}
	callback_http_request(req, args, content_type,msg);


	if (lws_add_http_header_status(wsi, 200, &p, end))
		return 1;
	if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_SERVER,
		(unsigned char *)"libwebsockets",
		13, &p, end))
		return 1;
	if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_ACCESS_CONTROL_ALLOW_ORIGIN,
		(unsigned char *)"*",
		1, &p, end))
		return 1;
	if (lws_add_http_header_by_token(wsi,
		WSI_TOKEN_HTTP_CONTENT_TYPE,
		(unsigned char *)content_type.c_str(),
		//(unsigned char *)"application/json",
		content_type.size(), &p, end))
		return 1;
	if (attachment)
	{
		if (lws_add_http_header_by_token(wsi,
			WSI_TOKEN_HTTP_CONTENT_DISPOSITION,
			(unsigned char *)attachment.c_str(),
			attachment.size(), &p, end))
			return 1;


	}

	if (lws_add_http_header_content_length(wsi,
		msg.size(), &p,
		end))
		return 1;
	if (lws_finalize_http_header(wsi, &p, end))
		return 1;

	*p = '\0';

	n = lws_write(wsi, buffer,
		p - (buffer),
		LWS_WRITE_HTTP_HEADERS);
	if (n < 0) {
		return -1;
	}
	/*
	* book us a LWS_CALLBACK_HTTP_WRITEABLE callback
	*/
	//strncpy((char*)buffer, msg.c_str(), msg.size());
	msg.insert(0, LWS_PRE, 'x');
	U8* data = (U8*)(LWS_PRE+ ((size_t)msg.c_str()));
	n = lws_write(wsi, data, msg.size(), LWS_WRITE_HTTP);

	return 0;
}
int LwsBase::callback_http_request(ctext req, z_strlist& args, z_string & content_type, z_string & output)
{
	ZTF;
	//content_type = "application/json";
	output = "{ data: [ 1,2,3,4,5 ] }";
	return 0;

}
int LwsBase::callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user,
	void *in, size_t len)
{
	UserContext *context = (UserContext *)user;
	LwsServerClient* client = 0;
	if (context)
		client = context->client;

	switch (reason) 
	{

	case LWS_CALLBACK_HTTP:
	{
		char buf[256];
		z_strlist args;
		int n;
		LWST("lws_http_serve: %s\n", in);

		//dump_handshake_info(wsi);

		/* dump the individual URI Arg parameters */
		n = 0;
		while (lws_hdr_copy_fragment(wsi, buf, sizeof(buf),
			WSI_TOKEN_HTTP_URI_ARGS, n) > 0) {
			args << buf;
			LWST("URI Arg %d: %s\n", n, buf);
			++n;
		}

		/*
		char name[100], rip[50];
		lws_get_peer_addresses(wsi, lws_get_socket_fd(wsi), name,
			sizeof(name), rip, sizeof(rip));
		sprintf(buf, "%s (%s)", name, rip);
		LWST("HTTP connect from %s\n", buf);
		*/
		ctext text_in = (ctext)in;
		if ((len < 1)
			|| (strcmp("/favicon.ico", text_in) == 0))
		{
			lws_return_http_status(wsi,
				HTTP_STATUS_BAD_REQUEST, NULL);
			goto try_to_reuse;
		}
#if 0 // TODO enable POST support
		/* if a legal POST URL, let it continue and accept data */
		if (lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI))
			return 0;
#endif
		json_request(text_in,wsi,  args);
		//lws_callback_on_writable(wsi);
		goto try_to_reuse;

#if 0
		/* this example server has no concept of directories */
		if (strchr((const char *)in + 1, '/')) {

			//lws_return_http_status(wsi, HTTP_STATUS_NOT_ACCEPTABLE, NULL);
			goto try_to_reuse;
		}

		if (!strncmp(text_in, "/postresults", 12)) {
			m = sprintf(buf, "<html><body>Form results: '%s'<br>"
				"</body></html>", pss->post_string);

			p = buffer + LWS_PRE;
			start = p;
			end = p + sizeof(buffer) - LWS_PRE;

			if (lws_add_http_header_status(wsi, 200, &p, end))
				return 1;
			if (lws_add_http_header_by_token(wsi,
				WSI_TOKEN_HTTP_CONTENT_TYPE,
				(unsigned char *)"text/html",
				9, &p, end))
				return 1;
			if (lws_add_http_header_content_length(wsi, m, &p,
				end))
				return 1;
			if (lws_finalize_http_header(wsi, &p, end))
				return 1;

			n = lws_write(wsi, start, p - start,
				LWS_WRITE_HTTP_HEADERS);
			if (n < 0)
				return 1;

			n = lws_write(wsi, (unsigned char *)buf, m, LWS_WRITE_HTTP);
			if (n < 0)
				return 1;

			goto try_to_reuse;
		}

#endif


		/* demonstrates how to set a cookie on / */
#if 0
		other_headers = leaf_path;
		p = (unsigned char *)leaf_path;
		if (!strcmp((const char *)in, "/") &&
			!lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_COOKIE)) {
			/* this isn't very unguessable but it'll do for us */
			gettimeofday(&tv, NULL);
			n = sprintf(b64, "test=LWS_%u_%u_COOKIE;Max-Age=360000",
				(unsigned int)tv.tv_sec,
				(unsigned int)tv.tv_usec);

			if (lws_add_http_header_by_name(wsi,
				(unsigned char *)"set-cookie:",
				(unsigned char *)b64, n, &p,
				(unsigned char *)leaf_path + sizeof(leaf_path)))
				return 1;
		}

		if (lws_is_ssl(wsi) && lws_add_http_header_by_name(wsi,
			(unsigned char *)
			"Strict-Transport-Security:",
			(unsigned char *)
			"max-age=15768000 ; "
			"includeSubDomains", 36, &p,
			(unsigned char *)leaf_path +
			sizeof(leaf_path)))
			return 1;
		n = (char *)p - leaf_path;
		other_headers = 0;
#endif
		n = 0;
	}
		break;

	case LWS_CALLBACK_HTTP_BODY:
		LWST("LWS_CALLBACK_HTTP_BODY: len %d\n", (int)len);
		break;

	case LWS_CALLBACK_HTTP_BODY_COMPLETION:
		LWST("LWS_CALLBACK_HTTP_BODY_COMPLETION\n");
		goto try_to_reuse;

	case LWS_CALLBACK_HTTP_FILE_COMPLETION:
		goto try_to_reuse;

	case LWS_CALLBACK_HTTP_WRITEABLE:
		LWST("LWS_CALLBACK_HTTP_WRITEABLE\n");

		goto try_to_reuse;


		/*
		* callback for confirming to continue with client IP appear in
		* protocol 0 callback since no websocket protocol has been agreed
		* yet.  You can just ignore this if you won't filter on client IP
		* since the default unhandled callback return is 0 meaning let the
		* connection continue.
		*/
	case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
		/* if we returned non-zero from here, we kill the connection */
		break;

	case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
		LWST("LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP\n");


		break;
	case LWS_CALLBACK_CLOSED_CLIENT_HTTP:
		LWST("LWS_CALLBACK_CLOSED_CLIENT_HTTP\n");
		return -1;
		break;
	case LWS_CALLBACK_RECEIVE_CLIENT_HTTP:
		LWST("LWS_CALLBACK_RECEIVE_CLIENT_HTTP: wsi %p\n", wsi);
		break;
	case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
		LWST("LWS_CALLBACK_RECEIVE_CLIENT_HTTP: wsi %p\n", wsi);
		break;
	case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
		LWST("LWS_CALLBACK_COMPLETED_CLIENT_HTTP\n");

		break;

		/*
		* callbacks for managing the external poll() array appear in
		* protocol 0 callback
		*/

	case LWS_CALLBACK_LOCK_POLL:
		break;

	case LWS_CALLBACK_UNLOCK_POLL:
		break;


	case LWS_CALLBACK_GET_THREAD_ID:
		/*
		* if you will call "lws_callback_on_writable"
		* from a different thread, return the caller thread ID
		* here so lws can use this information to work out if it
		* should signal the poll() loop to exit and restart early
		*/

		/* return pthread_getthreadid_np(); */

		break;


	default:
		break;
	}

	return 0;

	/* if we're on HTTP1.1 or 2.0, will keep the idle connection alive */
try_to_reuse:
	if (lws_http_transaction_completed(wsi))
		return -1;

	return 0;
}
#endif