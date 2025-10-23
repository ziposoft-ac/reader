/*________________________________________________________________________

 z_string_h

________________________________________________________________________*/


#ifndef z_string_h
#define z_string_h


#include "zipolib/z_stream.h"


class z_strlist;
class z_string : public std::string, public z_stream
{
public:
	z_string(ctext x)
	{
		if (x) assign(x);
	}
	z_string(const std::string& s)
	{
		assign(s);
	}
	z_string(const z_string& s)
	{
		assign(s);
	}
	virtual ~z_string()
	{

	}
	z_string(int i);
	z_string(const z_strlist& list) ;
    z_string() : std::string()  { }
    z_string & operator = (int inval);
    z_string & operator = (char c);
	z_string & operator = (const bool b);
	template<class T>  z_string & operator = (const T& var) { assign(std::to_string(var));  return *this; };
	z_string & operator = (const char* str);
	z_string & operator = (char* str);
	z_string & operator = (const z_string& s);
	z_string & operator = (const z_strlist& s);
	z_string & operator = (const std::string s) { assign(s); return *this; }

	bool contains(ctext text);
	bool contains(char c);
	//const char operator[](size_t offset){	return at(offset);	}

	size_t split(const char delemiter, z_strlist & list) const;
	size_t split(size_t pos, z_string & s1, z_string & s2);
	size_t split_pair(const char delemiter, z_string & s1, z_string & s2);


	//z_string & operator << (const std::ifstream& s);


	operator ctext() const
	{
		return this->c_str();
	}
	operator bool()
	{
		return size() != 0;
	}
	virtual z_status write(const char* data, size_t len) override;
	virtual z_status write_safe(const char* data, size_t len) override;
	virtual void write_std(const std::string& x) override;
	z_string & format(ctext pFormat, ...);

	void to_upper();

	void make_filename();
	void trim_leading_underscore()
	{
		if (size() > 1)
			if (at(0) == '_')
				erase(0, 1);
	}
	I64 get_i64_val();
	int get_int_val() const;
	U64 get_u64_val();
	double get_double_val();
	template <class T > T  get_as_def (T def);


	z_string& copy_escaped(ctext input);
};
int memsearch(const char* hay, int haysize, const char* needle, int needlesize);

void z_str_escape(ctext in, std::string& _out);
void z_str_unescape(std::string& in, std::string& _out);
extern const z_string g_null_string;
#endif

