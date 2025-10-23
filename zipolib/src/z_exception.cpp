#include "pch.h"
#include "zipolib/z_exception.h"
#include "zipolib/z_error.h"
#include "zipolib/z_strlist.h"


void z_except::point::debug_log()
{

	
	get_zlog().error_msg_t(_pm._status, z_log::lvl_error, _pm._file, _pm._func, _pm._line,_msg.c_str());


}

z_except::point* z_except::_addline(params pm)
{

	point* p = z_new point(pm);
	_trace.push_back(p);
	return p;
}

void z_except::add(params pm, const char* lpszFormat, ...)
{
	point* p = _addline(pm);
	if (lpszFormat)
	{
		va_list ArgList;
		va_start(ArgList, lpszFormat);
		p->_msg.format_args(lpszFormat, ArgList);
		va_end(ArgList);
	}
	p->debug_log();
}

z_except::z_except(params pm, const char* lpszFormat, ...)
{
	_status = pm._status;
	point* p = _addline(pm);
	if (lpszFormat)
	{
		va_list ArgList;
		va_start(ArgList, lpszFormat);
		p->_msg.format_args(lpszFormat, ArgList);
		va_end(ArgList);
	}
	p->debug_log();

}
z_string& z_except::get_message()
{
	static z_string unknown = "No error message available";
	if (_trace.size())
	{
		return _trace[0]->_msg;


	}

	return unknown;


}
void z_except::dump_to_stream(z_stream& s)
{
	for (auto& p : _trace)
	{
		p->dump(s);
	}
}
void z_except::dump_to_array(z_strlist& list)
{
	for (auto& p : _trace)
	{
		z_string s;
		p->dump(s);
		list << s;
	}
}