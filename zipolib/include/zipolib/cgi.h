#ifndef z_cgi_h
#define z_cgi_h
#include "zipolib.h"
#include "z_factory_controller.h"
#include <unordered_map>


#ifdef BUILD_ZBSITE
#define MAX_DATABASE_FILE_SIZE 9999 //In MB
#define MAX_UPLOAD_FILE_SIZE 100 //In MB
#define OPT_CREATE_DB_DIR false
#else 

#ifdef BUILD_ZBSITE_DEMO
#define MAX_DATABASE_FILE_SIZE 6 //In MB
#define MAX_UPLOAD_FILE_SIZE 5 //In MB
#define OPT_CREATE_DB_DIR false
#else


// BUILD_DIST - default config
#define MAX_DATABASE_FILE_SIZE 9999 //In MB
#define MAX_UPLOAD_FILE_SIZE 100 //In MB
#define OPT_CREATE_DB_DIR true

#endif//BUILD_ZBSITE_DEMO
#endif//BUILD_ZBSITE

#define MAX_CONTENT_LENGTH   (MAX_UPLOAD_FILE_SIZE*0x100000+0x1000) //in bytes

class dummyz
{
public:
	z_string x;
	dummyz()
	{
		x = "fred";
	}
};
class cgi_input;
class zf_cc_cgi : public zf_command_context
{
public:
	zf_cc_cgi(cgi_input* fc);
	z_json_stream* _json_stream = 0;
	void set_cc(zf_node& exec_node, zf_operation oper, z_json_stream& stream)
	{
		_node = exec_node;
		_operation = oper;
		_json_stream = &stream;
	}
	z_json_stream* get_json_stream() override
	{
		if (!_json_stream)
			Z_THROW("json stream not set");
		return _json_stream;
	}
};
class cgi_input : public z_factory_controller
{

	zp_text_parser _parser;
	 
	bool _bSimulate;
public:
	bool _bSaveContext;
	bool _dumpenv=false;
	
	const char* _contextFile = "cgidebug.txt";

	cgi_input()
	{

		_bSimulate = false;
		_bSaveContext = true;
		//_contextFile = "cgi_context.txt";

	}
	virtual ~cgi_input();

	//why unordered?
	//std::unordered_map<std::string, std::string> _query;
	z_stl_map<z_string, z_string> _query;

	/*
	Parses CGI input. 

	*/
	z_status parse_input();
	/*
	Returns true if in CGI
	Returns false if not in CGI mode

	*/
	bool is_cgi_mode(bool debugReplay);
	z_status env_load();

	z_status parseGetStr(z_string &s);
	z_status parseGetStr(ctext str, size_t len);

	z_status parse_content();
	z_status parsePostMultiPart(std::istream& is);

	z_string _HTTP_REFERER;
	z_string _HTTP_USER_AGENT;
	z_string _REMOTE_HOST;
	z_string _REQUEST_METHOD;
	z_string _CONTENT_TYPE;
	z_string _CONTENT_LENGTH;
	z_string _REMOTE_ADDR;
	z_string _QUERY_STRING;
	z_string _HTTP_COOKIE;
	z_string _SCRIPT_NAME;
	z_string _SCRIPT_ROOT;
	z_string _PATH_TRANSLATED;
	z_string _PATH_INFO;
	z_string _REQUEST_URI;
	z_string _REMOTE_IDENT;
	z_string _REMOTE_USER;
	
	void dumpQuery();
	z_status ExecQuery(z_json_stream& output);
	virtual zf_node& get_self()
	{
		if (!_self)
			_self.init((z_void_obj*)this, GET_FACT(cgi_input));
		return  _self;
	}
	virtual z_status evaluate_feature(zf_node& o, z_string& name)  { return Z_ERROR_NOT_IMPLEMENTED; }
	z_status get_obj_features_json(z_json_stream& s, z_json_obj& params);
	virtual void init_commands();

};

#endif
