#ifndef z_exception_h
#define z_exception_h
#include "zipolib/zipo.h"
#include "zipolib/z_string.h"
#include "zipolib/z_vector.h"

#define EXCEPTION_TRACE 1
#ifdef EXCEPTION_TRACE

#define Z_THROW_S(status) throw z_except({status,__FILE__,__FUNCTION__,__LINE__})
#define	Z_THROW_MSG(status,...)  throw z_except({status,__FILE__,__FUNCTION__,__LINE__},__VA_ARGS__)
#define	Z_THROW(...)  throw z_except({zs_unknown_error,  __FILE__,__FUNCTION__,__LINE__},__VA_ARGS__)
#define	Z_EXCEPT_ADD(_e_,...)  _e_.add({zs_unknown_error,__FILE__,__FUNCTION__,__LINE__},__VA_ARGS__)
#define	Z_RETHROW(...)  ex.add({zs_unknown_error,__FILE__,__FUNCTION__,__LINE__},__VA_ARGS__);throw
#else
#define Z_THROW(status) throw z_except({_status_})
#define Z_THROW_S(status) throw z_except({_status_})

#define	Z_THROW_MSG(status,...)  throw z_except({_status_},__VA_ARGS__)
#define	Z_EXCEPT_ADD(_e_,_status_,...)  _e_.add({_status_},__VA_ARGS__)
#endif
#define	Z_THROW_NOT_SUPPORTED   Z_THROW_MSG(zs_operation_not_supported,"Operation is not supported")
class z_except 
{
public:
	struct params
	{
		z_status _status;
#ifdef EXCEPTION_TRACE
		ctext _file;
		ctext _func;
		int _line;
#endif
	};
private:
	z_status _status;

	class point
	{
	public:
		point(z_except::params pm)
		{
			_pm = pm;
			
		}
		params _pm;
		z_string _msg;
		void dump(z_stream& s)
		{
			s.format_append("%s(%d) %s : %s\n", z_get_filename_from_path( _pm._file), _pm._line, _pm._func, _msg.c_str());
		}
		void debug_log();
	};

	point* _addline(params pm);

	z_obj_vector<point> _trace;

public:
	z_except()
	{

	}
	virtual ~z_except()
	{

	}
	z_string& get_message();
	z_status get_status() { return _status;  }
	void add(params pm, const char* lpszFormat, ...);
	z_except(params pm)
	{
		_status = pm._status;
		point* p = _addline(pm);
	}
	z_except(params pm, const char* lpszFormat, ...);
	void dump_to_stream(z_stream& s);
	void dump_to_array(z_strlist& list);
};



#endif

