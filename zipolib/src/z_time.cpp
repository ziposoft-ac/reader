#include "pch.h"

#include "zipolib/z_time.h"
#include "zipolib/z_error.h"
#include "zipolib/z_strlist.h"
#include <ctime>
#include <time.h>


U64 z_time_get_ticks();
double z_time_get_ms_elapsed(U64 ticks_start);





#ifdef WINDOWS


U64 z_time_get_ticks()
{
	LARGE_INTEGER start_time;
	QueryPerformanceCounter(&start_time);
	return  start_time.QuadPart;
}

double z_time_get_ms_elapsed(U64 start_time) //millisec
{
	static LARGE_INTEGER freq = { 0,0 };
	double dt;
	U64 elapsed_tics = z_time_get_ticks() - start_time;
	if (!(freq.QuadPart))
		QueryPerformanceFrequency(&freq);
	dt = (double)elapsed_tics;
	dt = dt * 1000;
	dt = dt / freq.QuadPart;
	return dt; //millisec

}

#else
#ifdef CLOCK_REALTIME
U64 z_time_get_ticks()
{
	timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	U64 milliseconds = t.tv_sec * 1000 + t.tv_nsec / 1000000;
	return milliseconds;
}

double z_time_get_ms_elapsed(U64 start_time) //millisec
{
	double dt;
	U64 elapsed_tics = z_time_get_ticks() - start_time;
	dt = (double)elapsed_tics;
	return dt; //millisec

}



#endif
#endif



#include <unordered_set>


/*=====================================================

				z_time

=====================================================*/
z_time::z_time()
{

	_t = 0;

}
z_time::~z_time()
{



}
z_time::z_time(U64 i)
{

	_t = i;

}
bool z_time::is_not_a_date_time() const
{

	return _t == 0;
}

#ifdef WIN32
extern "C" char* strptime(const char *buf, const char *format, struct tm *tm);
#endif 
void convert_iso8601(const char *time_string, int ts_len, struct tm *tm_data)
{

	char temp[64];
	memset(temp, 0, sizeof(temp));
	strncpy(temp, time_string, ts_len);
}



z_time& z_time::set_from_iso8601_string(const z_string& iso_string)
{

	struct tm ctime;
	memset(&ctime, 0, sizeof(struct tm));
	strptime(iso_string.c_str(), "%FT%T%z", &ctime);

	set_tm(ctime,false);
	return *this;
}
z_time& z_time::set_from_iso_string(const z_string& iso_string)
{
	try {
//_pt = posix_time::from_iso_string(iso_string);
        Z_ERROR_NOT_IMPLEMENTED;

	}
	catch (...)
	{
		Z_ERROR_MSG(zs_bad_parameter, "bad time");
	}
	
	return *this;
}
bool z_time::set_time_only(const z_string& time_str,bool local)
{
	z_strlist list;
	if (time_str.split(':', list) != 3)
		return false;
	tm tm_struct;
	get_tm(tm_struct,local);
	tm_struct.tm_hour = list[0].get_int_val();
	tm_struct.tm_min = list[1].get_int_val();
	tm_struct.tm_sec = list[2].get_int_val();
	set_tm(tm_struct,local);
	return true;
}


z_string z_time::to_iso_string() const
{
	z_string zs;
	try
	{
        Z_ERROR_NOT_IMPLEMENTED;

        //zs = posix_time::to_iso_string(_pt);
	}
	catch (...)

	{
		Z_ERROR_MSG(zs_bad_parameter, "bad time");

		zs = "bad time";
	}

	return zs;
}

z_time& z_time::set_tm(tm tm_struct,bool local)
{
	try {
        _t=mktime(&tm_struct)*1000;
        if(!local)
            _t=_t-timezone*1000;
	}
	catch (...)
	{
		Z_ERROR_MSG(zs_bad_parameter, "Error converting to time struct");

	}
	return *this;
}
void z_time::get_tm(tm& tm_struct,bool local) const
{
    static bool tz_is_set=false;
    if(!tz_is_set)
    {
        tzset();
        tz_is_set=true;
    }
	try {
	    time_t t=(time_t)(_t/1000);
	    if(local)
	        localtime_r(&t,&tm_struct);

	    else
	        gmtime_r(&t,&tm_struct);

	}
	catch (...)
	{
		Z_ERROR_MSG(zs_bad_parameter,"Error converting to time struct");
	}

}
/*
This gets the number of milliseconds elapsed since the value of this time
=now()-this
*/
U64 z_time::get_elapsed_ms() const
{
	auto milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();


    return milliseconds_since_epoch-_t;

}
// This gets the number of milliseconds elapsed since the epoch 
U64  z_time::get_ptime_ms() const
{
    return _t;

}
// This gets the number of seconds elapsed since the epoch 
U32  z_time::get_ptime_seconds() const
{
    return _t/1000;
}
z_time& z_time::set_ptime_ms(U64 ms_ptime)
{
	//_pt = posix_time::from_time_t(ms_ptime / 1000) + posix_time::milliseconds(ms_ptime % 1000);
	return *this;
}
z_time& z_time::set_now()
{
	auto milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    _t =    milliseconds_since_epoch;
	return *this;
}
U64 z_time::get_now_ms()  {
	auto milliseconds_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	return   (U64) milliseconds_since_epoch;
}
z_status z_time::string_format(z_string & s, ctext format,bool local) const
{
	tm tm_struct;
	get_tm(tm_struct,local);
	char buff[40];
	strftime(buff, 39, format, &tm_struct);
	s = buff;
	return zs_ok; //TODO -check errors!
}

bool z_time::operator >(const z_time& t2) const
{
	return _t > t2.get_t();
}

z_time & z_time::operator = (U64 val)
{
	_t=val;
	return *this;
}

U32 z_time::get_fract_ms() const
{
	return get_ptime_ms() % 1000;

}

z_string z_time::getTimeStrLocalFsFormat() {
	z_time t;
	z_string  s;

	t.set_now();
	t.string_format(s, "%Y_%m_%d_%H_%M_%S",true);

	return s;

}

z_string z_time::to_string_ms(bool local) const
{
	z_string  s;
	if (is_not_a_date_time())
		return "bad time";
	if (_t == 0)
		return "bad time";

	try
	{
	    string_format(s, "%H:%M:%S",local);
		s.format_append(".%03d", get_fract_ms());

	}
	catch (...)
	{
		Z_ERROR_MSG(zs_bad_parameter, "bad time");
	}
	return s;
}

z_string z_time::to_readable_string(bool local) const
{
	z_string  s;
	if (is_not_a_date_time())
		return "bad time";
	if(_t == 0)
		return "bad time";

	try
	{
	    string_format(s, "%c",local);
	}
	catch (...)
	{
		Z_ERROR_MSG(zs_bad_parameter, "bad time");
	}
	return s;
}
z_time z_time::get_now_local()
{
	z_time t;
	t.set_now();
	return t;
}
z_string z_time::getTimeStrLocal()
{
    z_time t;
    t.set_now();
    return t.to_readable_string(true);
}
z_string z_time::getTimeStrGmt()
{
    z_time t;
    t.set_now();
    return t.to_readable_string(false);
}
z_time z_time::get_now()
{
	z_time t;
	t.set_now();
	return t;
}
std::ostream & operator<<(std::ostream &os, const z_time& td)
{
	try
	{
		os << td.get_t();
	}
	catch (...)
	{
		os << " ERROR converting time";
	}
	return os;
}
z_stream & operator<<(z_stream &os, const z_time& td)
{
	try
	{
		os << td.to_iso_string();
	}
	catch (...)
	{
		os << " ERROR converting time";
	}
	return os;
}
/*=====================================================

z_time_duration

=====================================================*/

z_time_duration::z_time_duration()
{
	_d = 0;

}
z_time_duration::z_time_duration(I64 val)
{
	_d = val;
}

z_time_duration::~z_time_duration()
{
}
z_time_duration z_time_duration::time_since(z_time& start)
{
	return z_time::get_now() - start;

}
z_time_duration& z_time_duration::set_from_string(const z_string& str) {
    z_strlist list;
    int smh[3] = {0,//sec
                  0,//min
                  0//hours
                   };
    _d = 0;
    int count = str.split(':', list);
    if(count<4)
    {
        for (int i = 0; i < count; i++)
            smh[2-i]=list[i].get_int_val();

        _d=(smh[0]+smh[1]*60+smh[2]*60*60)*1000;
    }




	return *this;
}
z_string z_time_duration::to_simple_string()
{
    z_string str;
    int h = (int)hours();
    int m = (int)minutes();
    int s = (int)seconds_part();
    //U64 f = (int)fractional_seconds();
    if (h)
        str.format("%02d:%02d:%02d", h, m, s);
    else
        str.format("%02d:%02d", m, s);
    return str;
}

z_time_duration z_time_duration::from_hours(long s)
{
	z_time_duration d;
    Z_ERROR_NOT_IMPLEMENTED;

    //get_boost_duration(d) = posix_time::hours(s);
	return d;
}
z_time_duration z_time_duration::from_minutes(long s)
{
	z_time_duration d;
    Z_ERROR_NOT_IMPLEMENTED;

	//	get_boost_duration(d) = posix_time::minutes(s);
	return d;
}
z_time_duration z_time_duration::from_seconds(long s)
{
	z_time_duration d;
    Z_ERROR_NOT_IMPLEMENTED;

	//	get_boost_duration(d) = posix_time::seconds(s);
	return d;
}
z_time_duration z_time_duration::milliseconds(long ms)
{
	z_time_duration d;
	d._d=ms;
	return d;
}


I64 z_time_duration::fractional_seconds() const
{
	return (int) (_d/100)%10;
}
I64 z_time_duration::seconds_part() const
{
	return (_d/1000)%60;
}
I64 z_time_duration::minutes() const
{
	return (_d/1000*60)%60;
}
I64 z_time_duration::hours() const
{
	return (_d/1000*60*60);
}
I64 z_time_duration::total_seconds() const
{
	return _d/1000;
}

I64 z_time_duration::total_milliseconds() const
{
	return _d;
}
z_string z_time_duration::format_hms() const
{
	z_string str;
	int h = (int)hours();
	int m = (int)minutes();
	int s = (int)seconds_part();
	U64 f = (int)fractional_seconds();
	if (h)
		str.format("%02d:%02d:%02d", h, m, s);
	else
		str.format("%02d:%02d", m, s);
	return str;

}
z_time z_time::operator-(const z_time_duration& duration) const
{
	z_time ztd;
    ztd._t = _t  - duration._d;
	return ztd;
}
z_time z_time::operator+(const z_time_duration& duration) const
{
	z_time ztd;
    ztd._t = _t  + duration._d;
	return ztd;
}
z_time_duration z_time::operator-( const z_time& t2) const
{
	z_time_duration ztd;
	ztd._d = _t - t2._t;
	return ztd;
}


std::ostream & operator<<(std::ostream &os, const z_time_duration& td)
{

	return os << td.format_hms();
}

void z_sleep_ms(int ms)
{
#ifdef WINDOWS
	Sleep(ms);
#else
	usleep(ms * 1000);
#endif

}
