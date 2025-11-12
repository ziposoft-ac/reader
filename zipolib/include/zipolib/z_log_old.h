#ifndef z_log_h
#define z_log_h

#ifdef  __cplusplus


#include "zipolib/zipo.h"
#include "zipolib/z_stream.h"

// TODO  - create "C" wrapper for this.


const U64 LOG_ERROR = 1;
const U64 LOG_WARN = 2;
const U64 LOG_CMDLINE = 4;
const U64 LOG_PARSE = 8;
const U64 LOG_CGI = 16;

class z_log
{
	z_file_out _file;
	
	int _indent;
public:

	z_log();
	enum z_log_level
	{
		lvl_error,
		lvl_warning,
		lvl_info,
		lvl_debug,
		lvl_trace,
	};
	void indent_inc();
	void indent_dec();

	void flush();
	z_log_level _level;
	U64 _mask;


    z_stream_error& get_stream_err();
	void set_stream_err(z_stream* s);
	void trace(ctext file, ctext func, int line);
	void trace_v(U64 mask, ctext file, ctext func, int line, ctext  lpszFormat, ...);
	z_status error_msg_t(z_status status, z_log_level lvl, ctext file, ctext func, int line, const char*  lpszFormat, ...);
	z_status error_t(z_status status, z_log_level lvl, ctext file, ctext func, int line);
	z_status fileout(ctext file_name);
    void add_stdout();
};
z_stream_multi& get_logger_dbg()
{
	return _stream_dbg;
}

z_stream& get_tracer();
z_log& get_zlog();
class z_trace_obj
{
public:
	z_trace_obj()
	{
		get_zlog().indent_inc();
	}
	~z_trace_obj()
	{
		get_zlog().indent_dec();

	}


};

#define ZLOG(...  ) get_zlog().get_stream_err().format_append(__VA_ARGS__);


#ifdef Z_TRACE_ENABLE 


#define	ZTF  get_zlog().trace( __FILE__,__FUNCTION__,__LINE__);
#define	ZT(...)  {get_zlog().trace_v(z_log::lvl_trace, __FILE__,__FUNCTION__,__LINE__,__VA_ARGS__);}
#define ZTS(_X_) { get_zlog().get_stream_dbg() << _X_; } 
#define Z_NEW_THREAD 
#define ZDBG(...  ) get_zlog().get_stream_dbg().format_append(__VA_ARGS__);
#define ZDBGS get_zlog().get_stream_dbg()
#else
#define	ZT(...) 
#define	ZTF
#define ZTS(_X_) 
#define ZDBG(...  ) 
#endif



extern "C"
{
	void  z_log_msg_f(const char*  lpszFormat, ...);
	void  z_log_trace(const char* file, const char* func, int line, bool endline);
}

#else
#define	ZTF  z_log_trace( __FILE__,__FUNCTION__,__LINE__,true);
#define	ZT(...)  {z_log_trace( __FILE__,__FUNCTION__,__LINE__,false); z_log_msg_f(__VA_ARGS__);z_log_msg_f("\n");}
void  z_log_msg_f(const char*  lpszFormat, ...);
void  z_log_trace(const char* file, const char* func, int line, bool endline);
#endif

#endif