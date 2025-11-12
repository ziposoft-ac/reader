#include "pch.h"
#include "zipolib/z_log.h"
#include "zipolib/z_strlist.h"
#include <stdarg.h>

z_stream_debug global_debug_stream;


z_status z_logger::create_file_out(ctext logname)
{
    z_status s;
    if (_file.is_open())
        return zs_already_open;
    s=_file.open(logname, "ab");
    if (s)
        return s;
    //_stream_dbg.add_stream(_file);
    add_stream(_file);
    return zs_ok;

}

z_logger_debug::z_logger_debug() {

    add_stream(global_debug_stream);
}
z_logger_debug& get_debug_logger() {
    static z_logger_debug debug;
    return debug;
}
z_logger_error::z_logger_error() {

    add_stream(global_debug_stream);
    add_stream(zout);
}
z_logger_default::z_logger_default() {

    add_stream(zout);
}
z_logger_error& get_error_logger() {
    static z_logger_error log;
    return log;
}

z_logger_default& get_default_logger() {
    static z_logger_default log;
    return log;
}

z_logger_trace::z_logger_trace() {
    add_stream(global_debug_stream);
}

z_logger_trace& get_trace_logger() {
    static z_logger_trace log;
    return log;
}


void  z_log_debug_f(const char*  lpszFormat, ...) {
    va_list ArgList;
    va_start(ArgList, lpszFormat);
    get_debug_logger().format_args(lpszFormat, ArgList);
    va_end(ArgList);
}
void  z_log_msg_f(const char*  lpszFormat, ...) {

}

void  z_log_trace(const char* file, const char* func, int line, bool endline) {

}


z_status z_log_warn_msg_t(z_status status,  ctext file, ctext func, int line, const char*  lpszFormat, ...) {
    return status;
}

z_status z_log_error_msg_t(z_status status,  ctext file, ctext func, int line, const char*  lpszFormat, ...) {
    return status;

}


z_status z_log_error_t(z_status status,  ctext file, ctext func, int line) {
    get_error_logger().trace_v(file,func,line,"ERROR: %s",zs_get_status_text(status));
    return status;

}


z_status z_log_warn_t(z_status status,  ctext file, ctext func, int line) {
    return status;

}
