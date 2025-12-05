
#ifndef z_log_h
#define z_log_h
#include "zipolib/zipo.h"
#include "zipolib/z_status.h"

#ifdef  __cplusplus

#include "zipolib/z_stream.h"

class z_logger : public z_stream_multi {
	z_file_out _file;

public:
	z_status create_file_out(ctext file_name);

};

class z_logger_default : public z_logger {
public:
	z_logger_default();


};
class z_logger_error : public z_logger {
public:
	z_logger_error();
};
class z_logger_trace : public z_logger {
	int _indent=0;

public:
	void indent_inc() { ++_indent; }
	void indent_dec() {	--_indent; }
	z_logger_trace();
};
z_logger_trace& get_trace_logger();
z_logger_error& get_error_logger();
z_logger_default& get_default_logger();

class z_trace_obj
{
public:
	z_trace_obj(const char* file, const char* func, int line)
	{
		get_trace_logger().trace_v(file,func,line,">>");
		get_trace_logger().indent_inc();
	}
	~z_trace_obj()
	{
		get_trace_logger().indent_dec();
		get_trace_logger().write_str("<<\n");

	}


};

#define	ZLOG(...) get_default_logger().format_append(__VA_ARGS__)

#define ZOUT(...) gz_stdout.format_append(__VA_ARGS__)


#ifdef DEBUG
class z_logger_debug : public z_logger {
public:
	z_logger_debug();

};
z_logger_debug& get_debug_logger();

#define ZDBGS get_debug_logger()
#define	ZDBG(...) get_debug_logger().format_append(__VA_ARGS__)
#define	ZDBG_HEX(data,len) get_debug_logger().dump_hex(data,len)
#endif

#ifdef Z_TRACE_ENABLE

#define	ZT(...)
#define	ZTF z_trace_obj ZTF_trace( __FILE__,__FUNCTION__,__LINE__);
#endif





extern "C"
{
#endif

// C/C++ functions


void  z_log_debug_f(const char*  lpszFormat, ...);
void  z_log_msg_f(const char*  lpszFormat, ...);
void  z_log_trace(const char* file, const char* func, int line, bool endline);

z_status z_log_warn_msg_t(z_status status,  ctext file, ctext func, int line, const char*  lpszFormat, ...);
z_status z_log_error_msg_return(z_status status,  ctext file, ctext func, int line, const char*  lpszFormat, ...);
void z_log_error_msg( ctext file, ctext func, int line, const char*  lpszFormat, ...);
z_status z_log_error_t(z_status status,  ctext file, ctext func, int line);
z_status z_log_warn_t(z_status status,  ctext file, ctext func, int line);
z_status z_debug_warn_t(z_status status,  ctext file, ctext func, int line);

#ifdef  __cplusplus
}

#else
// C only

#ifdef DEBUG
#define	ZDBG(...) z_log_debug_f(__VA_ARGS__)
#define	ZDBG_HEX(data,len)
#endif

#endif





#ifndef DEBUG
#define	ZDBG(...)
#define ZDBGS  gz_stream_null
#define	ZDBG_HEX(data,len)

#endif

#ifndef Z_TRACE_ENABLE
#define	ZT(...)
#define	ZTF
#endif





#endif



