/*________________________________________________________________________

 z_stream_h

________________________________________________________________________*/


#ifndef z_stream_h
#define z_stream_h

#include "zipolib/z_status.h"
#include "zipolib/z_vector.h"
class z_string;
ctext z_get_filename_from_path(ctext fullpath);
class z_stream
{
	/*
	z_string uses this class, so NO member variables!
	*/

protected:
	virtual z_status write_safe(const char* data, size_t len);

public:
	z_stream();
	virtual ~z_stream();

	//z_stream(const z_stream& stream);

	z_stream & operator % (int tabs) { indent(tabs); return *this; }

	z_stream & operator << (const std::string& x);
	z_stream & operator << (const z_string& x);
	z_stream & operator << (char x);
	z_stream & operator << (const char* x);
	z_stream & operator << (U16 x);
	z_stream & operator << (int32_t x);
	z_stream & operator << (int64_t x);
	z_stream & operator << (uint32_t x);
	z_stream & operator << (uint64_t x);
	z_stream & operator << (float x);
	z_stream& operator << (long double x);
	z_stream & operator << (double x);
	z_stream & dump_hex(U8* data,size_t len);
	z_stream & format_append(ctext pFormat, ...);
	z_stream & format_line_with_time(ctext pFormat, ...);
	bool format_args(ctext pFormat, va_list ArgList);
	int try_format_args(int buff_size, ctext pFormat, va_list ArgList);

	void indent(int tabs);

	template<class T> z_stream& operator , (T x)
	{
		*this << ',';
		// TODO // *this << '\"' << x << '\"';
		*this  << x ;
		return *this;
	}
	z_stream & operator , (double x);
	virtual z_status write(const char* data, size_t len) = 0;
	virtual void write_std(const std::string& x);
	virtual z_status write_str(const char* data, size_t len = -1);
	void trace(ctext file, ctext func, int line, bool endline);
	void trace_v(ctext file, ctext func, int line,  ctext pFormat, ...);
	void trace_vargs(ctext file, ctext func, int line, ctext pFormat, va_list ArgList);
	void time_mark(U64 elap_ms);


	virtual void flush() {}
};
class z_stream_null : public z_stream
{
public:
	virtual z_status write(const char* data, size_t len) { return zs_ok; }
	virtual z_status write_str(const char* data, size_t len = -1) { return zs_ok; }

	template<class T> z_stream& operator , (T x)
	{
		return *this;
	}
	template<class T> z_stream& operator << (T x)
	{
		return *this;
	}
	z_stream_null() {}
	virtual ~z_stream_null() {}

};
extern z_stream_null gz_stream_null;

class z_file_out : public z_stream
{
	FILE* _file;
public:
	virtual z_status write(const char* data, size_t len);
	virtual z_status write_str(const char* data, size_t len = -1);
	virtual void flush();
	z_file_out(ctext filename);
	z_file_out();
	virtual ~z_file_out();
	z_status open(ctext filename, ctext mode = "wb");
	void close();
	bool is_open();

};
#ifdef WINDOWS
class z_stream_windbg : public z_stream
{
	std::mutex _mutex;
public:
	virtual z_status write(const char* data, size_t len);
	virtual z_status write_str(const char* data, size_t len = -1);


};
#endif

class z_stream_stdout : public z_stream
{
public:
	virtual z_status write(const char* data, size_t len);
	virtual void flush();
};
class z_stream_debug: public z_stream
{
    int _fPipe=0;
	bool _add_time_mark=true;
    z_status open();
public:

    virtual z_status write(const char* data, size_t len);
    virtual void flush();
};
extern z_stream_stdout gz_stdout;
class z_stream_stderr : public z_stream
{
public:
	virtual z_status write(const char* data, size_t len);
	virtual void flush();
};

extern z_stream_stderr gz_stderr;

class z_stream_multi : public z_stream
{
public:
	z_obj_vector<z_stream,false> _streams;
	void add_stdout()
	{
		_streams.add(&gz_stdout);
	}
	virtual z_status write(const char* data, size_t len);
	virtual void flush();

	void add_stream(z_stream& stream)
	{
			_streams.add_unique(&stream);
	}
};

class z_stream_error : public z_stream_multi
{
public:
	z_stream_error();

};

extern z_stream_error gz_stream_error;

#endif

