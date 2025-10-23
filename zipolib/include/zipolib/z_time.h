/*________________________________________________________________________

 z_time_h

________________________________________________________________________*/


#ifndef z_time_h
#define z_time_h

#include "zipolib/z_status.h"
#include "zipolib/z_string.h"


class z_time_duration;
class z_time
{
	friend z_time_duration;
private:
	U64 _t;// milliseconds past the
public:
	z_time();
	~z_time();

	// Set this z_time to now
	z_time& set_now();
	z_time& set_ptime_ms(U64 ms_ptime);

	static U64 get_now_ms();

	// This gets the number of milliseconds since the epoch
	U64 get_ptime_ms() const;

	// This gets the number of milliseconds elapsed since the time
	U64 get_ms_since() const;
	// This gets the number of seconds elapsed since the epoch 
	U32 get_ptime_seconds() const;

	z_time& set_tm(tm tm_struct,bool local);
	void get_tm(tm& tm_struct,bool local) const;

	z_time& set_from_iso8601_string(const z_string& iso_string);
	z_time& set_from_iso_string(const z_string& iso_string);
	z_string to_iso_string() const;
	z_string to_readable_string(bool local=true) const;
	z_string to_string_ms(bool local=true) const;
	z_status string_format(z_string & s,ctext format,bool local) const;

	U32 get_fract_ms() const;


	bool set_time_only(const z_string& time,bool local);

	bool is_not_a_date_time() const;

	// Static func to get z_time set to now
	static z_string getTimeStrLocal();
	static z_string getTimeStrGmt();

	static z_time get_now();
	// Static func to get z_time set to now
	static z_time get_now_local();
	// todo this is used for durations???
	operator U64() const { return _t; }
	U64 get_t() const { return _t; }


	/*
	Check if it is a valid time
	*/
	operator bool() const	{		return _t!=0;	}
	void invalidate() { _t = 0; }


	/* 
	This gets the number of milliseconds elapsed since the value of this time
	=now()-this
	*/
	U64 get_elapsed_ms() const;

	bool operator > (const z_time& t2) const;
	z_time_duration operator-(const z_time& t2) const;
	z_time operator-(const z_time_duration& t2) const;
	z_time operator+(const z_time_duration& t2) const;
	friend std::ostream & operator<<(std::ostream &os, const z_time& td);
	friend z_stream & operator<<(z_stream &os, const z_time& td);
    z_time & operator = (U64 val);

};
class z_time_duration
{
	friend z_time;
private:
	I64 _d; // milliseconds
public:
	z_time_duration();
	z_time_duration(I64 val);
	~z_time_duration();
	friend std::ostream & operator<<(std::ostream &os, const z_time_duration& td);
	z_time_duration operator *(int multiplier) 
	{
		_d *= multiplier;
		return *this;
	}
	operator I64() const
	{
		return _d;
	}
	z_string to_simple_string();
	z_time_duration& set_from_string(const z_string& str);

	static z_time_duration from_hours(long s);
	static z_time_duration from_minutes(long s);
	static z_time_duration from_seconds(long s);
	static z_time_duration milliseconds(long ms);

	I64 total_milliseconds() const;
	I64 total_seconds() const;
	I64 fractional_seconds() const;
	I64 seconds_part() const;
	I64 minutes() const;
	I64 hours() const;

	void  set_zero() {_d = 0; }
	bool is_zero() { return _d == 0; }
	bool is_non_zero() { return _d != 0;  }
	static z_time_duration time_since(z_time& start);

	z_string format_hms() const;
	
};


std::ostream & operator<<(std::ostream &os, const z_time_duration& td);
std::ostream & operator<<(std::ostream &os, const z_time& zt);
void z_sleep_ms(int ms);


#if 0
class z_timer_service;

typedef bool (*z_timer_callback)(void* data);

class z_timer
{
	class _private_data;
	friend z_timer_service;
private:
	_private_data* _pd;

	int _ms;
	z_timer_service* _service;
	z_timer_callback _user_callback;
	void* _user_data;
public:
	void _callback();

	z_timer(z_timer_service *service, z_timer_callback callback,int ms, void* user_context);
	virtual ~z_timer();

	void start();
	void stop();
	void start(int ms);
	void restart(int ms);

};
class z_timer_service
{
	class _private_data;
	friend z_timer;
private:
	_private_data* _pd;
	void start_if_necessary();

	bool _started;
public:

	z_timer_service();
	virtual ~z_timer_service();
	void quit();
	bool start_and_block();
	bool start();
	void run_thread();
	z_timer* create_timer_ms(int ms, z_timer_callback callback, void* user_data, bool start = true);
	// TODO timer callback for classes
	template <typename T>  z_timer* create_timer_t(int ms, z_timer_callback callback, void* user_data, bool start = true);

};
#endif
U64 z_time_get_ticks();
double z_time_get_ms_elapsed(U64 ticks_start);


#endif

