#include "pch.h"

#include "zipolib/z_log.h"
#include "zipolib/z_strlist.h"

#include <stdarg.h>


z_stream_multi _stream_dbg;
z_stream_error _stream_err;


z_log& get_zlog()
{
	static z_log _log;
	return _log;

}

z_log::z_log()
{
	_multi_thread = false;
	_level = lvl_error;
	_indent = 0;

	_stream_dbg.add_stream(_debug_stream);
	_stream_err.add_stream(_debug_stream);

}

void z_log::flush()
{

	_stream_dbg.flush();
	_stream_err.flush();
	_file.flush();
	//_stdout.flush();
}

void z_log::indent_inc()
{
	_indent++;
}
void z_log::indent_dec()
{
	if(_indent>0)
		_indent--;
}

std::mutex _trace_mutex;
void z_log::add_stdout()
{
    _stream_dbg.add_stdout();
    _stream_err.add_stdout();
}
void z_log::set_stream_err(z_stream* s)
{
	//_stream_err = s;
}
void  z_log::trace(ctext file, ctext func, int line)
{
	std::unique_lock<std::mutex> mlock(_trace_mutex);

	_stream_dbg << z_get_filename_from_path(file) << '(' << line << ") " << func << "()";
	_stream_dbg << "\n";

}
void z_log::trace_v(U64 mask, ctext file, ctext func, int line, ctext pFormat, ...)
{
	if (!(mask&_mask))
		return;
	std::unique_lock<std::mutex> mlock(_trace_mutex);

	_stream_dbg << z_get_filename_from_path(file) << '(' << line << ") " << func << "()";
	static uint buff_size = 80;
	va_list ArgList;
	int i = 1;
	int result;
	while (i--)
	{
		va_start(ArgList, pFormat);
		result = _stream_dbg.try_format_args(buff_size,pFormat, ArgList);
		va_end(ArgList);
		if (result == 0)
			break;
		buff_size = result;
	}
	_stream_dbg << "\n";

}

z_stream_error& z_log::get_stream_err()
{
	return _stream_err;
}
z_status z_log::fileout(ctext logname)
{
	z_status s;
	if (_file.is_open())
		return zs_already_open;
	s=_file.open(logname, "ab");
	if (s)
		return s;
	//_stream_dbg.add_stream(_file);
	_stream_err.add_stream(_file);
	return zs_ok;

}


z_status z_log::error_msg_t(z_status status, z_log_level lvl, ctext file, ctext func, int line, const char*  lpszFormat, ...)
{

	_stream_err.trace(file, func, line, false);
	_stream_err << "ERROR: " << zs_get_status_text(status) << " - ";


	if (lpszFormat)
	{
		va_list ArgList;
		va_start(ArgList, lpszFormat);
		_stream_err.format_args(lpszFormat, ArgList);
		va_end(ArgList);
	}
	_stream_err << "\n";
	return status;

}

z_status z_log::error_t(z_status status, z_log_level lvl, ctext file, ctext func, int line)
{
	_stream_err.trace(file, func, line, false);
	_stream_err << "ERROR: " << zs_get_status_text(status) << "\n";

	return status;

}
void  z_log_trace(ctext file, ctext func, int line,bool endline)
{
	get_zlog().get_stream_dbg().trace(file, func, line, endline);

}
void z_log_msg_f( const char*  lpszFormat, ...)
{

	va_list ArgList;
	va_start(ArgList, lpszFormat);
	get_zlog().get_stream_dbg().format_args(lpszFormat, ArgList);
	va_end(ArgList);


}
