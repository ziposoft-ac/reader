/*________________________________________________________________________

 z_string_h

________________________________________________________________________*/


#ifndef z_stream_json_h
#define z_stream_json_h


#include "zipolib/z_string.h"


class z_json_stream : public z_stream
{
	bool _pretty_print;
	bool _pretty_print_array;
	U32 _depth;
	bool _need_comma;

	z_stream &_out;

	void down();
	void up();
	virtual z_status write(const char* data, size_t len);
	virtual void write_std(const std::string& x) override;
public:
	z_json_stream(z_stream& s, bool pretty_print = false);
	virtual ~z_json_stream();

	void reset(bool pretty_print = false);
	void key(ctext name);
	void val(ctext name);
	void val_str_array(z_strlist& list);
	void keyval_int(ctext name, I64 val);
	void keyval(ctext name,ctext val);
	void key_bool(ctext name,bool val);
	void val_end();
	void obj_start();
	void obj_val_start(ctext key);
	void obj_end();
	void array_start();
	void array_end();
	void obj_array_start();
	void obj_array_end();
	// returns previous value, for save/restore
	bool set_pretty_print(bool on);
	void indent_depth() { z_stream::indent(_depth); }

	template<class T> z_json_stream& operator , (T x)
	{
		*this << ',' << x;
		return *this;
	}

	z_json_stream & operator % (const z_string & x);
	z_json_stream & operator % (const char* x);
	z_json_stream& operator % (U32 x);
	z_json_stream & operator % (int x);
	z_json_stream & operator % (bool x);
	z_json_stream & operator % (double x);
	z_json_stream & operator % (int64_t x);
	

	static z_string escape(ctext txt);

};
extern z_json_stream stdout_json;

class z_json_str_stream : public z_json_stream
{
    z_string _s;
public:
    z_json_str_stream() : z_json_stream(_s)
    {

    }
    z_string& as_string() { return  _s; }
};
#endif

