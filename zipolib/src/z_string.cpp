#include "pch.h"
#include "zipolib/z_strlist.h"
#include "zipolib/z_error.h"
#include "zipolib/ptypes_cset.h"
#include <memory>
#include <stdarg.h>
using namespace std;
/*
utf8 test = u8"∃y ∀x ¬(x ≺ y)";
utf8 smile = u8"😃";
*/

/* binary search in memory */
int memsearch(const char* hay, int haysize, const char* needle, int needlesize) {
	int haypos, needlepos;
	haysize -= needlesize;
	for (haypos = 0; haypos <= haysize; haypos++) {
		for (needlepos = 0; needlepos < needlesize; needlepos++) {
			if (hay[haypos + needlepos] != needle[needlepos]) {
				// Next character in haystack.
				break;
			}
		}
		if (needlepos == needlesize) {
			return haypos;
		}
	}
	return -1;
}


const z_string g_null_string;

z_string::z_string(int i)
{

	assign(std::to_string(i));
}


z_string::z_string(const z_strlist& list )
{
	list.get_as_string(*this);
}
z_string & z_string::operator = (char* str)
{
	if (str)
		assign(str);
	return *this;
}
z_string & z_string::operator = (const char* str)
{
	if(str)
	assign(str);
	return *this;
}
z_string & z_string::operator = (const bool b)
{
	*this = (b ? "true" : "false");
	return *this;

}
z_string & z_string::operator = (int i)
{
    assign(std::to_string(i));
    return *this;

}
z_string & z_string::operator = (char c)
{
	assign(1,c);
	return *this;

}
z_string & z_string::operator = (const z_string& s)
{
	assign(s);
	return *this;

}
z_string & z_string::operator = (const z_strlist& list)
{
	list.get_as_string(*this);
	return *this;

}
bool z_string::contains(char c)
{
    return find(c)!=-1;
}
bool z_string::contains(ctext text)
{
	return find(text)!=-1;
}
z_status z_string::write_safe(const char* data, size_t len)
{
	Z_ASSERT(len != 0);
	Z_ASSERT(data);
	Z_ASSERT(data[len - 1] != 0);
	append(data, len);
	return zs_ok;
}
void z_string::write_std(const std::string& x)
{
	std::string::append(x);
}

z_status z_string::write(const char* data, size_t len)
{

	if (len == 0)
		return zs_ok;
	if (!data)
		return Z_ERROR(zs_bad_parameter);
	if (data[len-1] == 0)
		len--;
	append(data, len);
	return zs_ok;
}

z_string & z_string::format(ctext pFormat, ...)
{
	clear();
	static uint buff_size = 80;
	va_list ArgList;
	int i = 2;
	int result;
	while (i--)
	{
		va_start(ArgList, pFormat);
		result = try_format_args(buff_size, pFormat, ArgList);
		va_end(ArgList);
		if (result == 0)
			break;
		buff_size = result;

	}
	return *this;


}

void z_string::to_upper()
{
	size_t i;
	for (i=0;i<length();i++)
	{
		z_string& s = *this;
		char c = s[i];
		if ((c >= 'a') && (c <= 'z'))
		{
			s[i] = c - ('a' - 'A');
		}



	}




}

void z_string::make_filename()
{
	cset bad("<>?\"\\/|*:");
	size_t i;
	for (i = 0; i<length(); i++)
	{
		z_string& s = *this;
		char c = s[i];
		if (c& bad)
		{
			s[i] = '_';
		}



	}
}

/*
z_string & z_string::format(ctext pFormat, ...)
{
	va_list ArgList;
	va_start(ArgList, pFormat);
	size_t size = vsnprintf(nullptr, 0, pFormat, ArgList) + 1; // Extra space for '\0'
	va_end(ArgList);
	//unique_ptr<char[]> buf(z_new char[size]);
	resize(size);
	va_start(ArgList, pFormat);
	//vsnprintf(buf.get(), size, pFormat, ArgList);
	vsnprintf((char*)data(), size, pFormat, ArgList);
	va_end(ArgList);
	resize(size-1);
	//assign(buf.get(), buf.get() + size - 1);

	return *this;
}
*/
size_t z_string::split(size_t pos, z_string & s1, z_string & s2)
{
	if (pos >= size())
	{
		s1 = *this;
		s2 = "";
		return 1;
	}
	s1 = substr(0, pos);
	s2 = substr(pos);
	return 2;

}

size_t z_string::split_pair(const char delemiter, z_string & s1, z_string & s2)
{
	size_t pos = 0;
	pos = find(delemiter);
	if (pos == string::npos)
	{
		s1 = *this;
		return 1;
	}
	s1 = substr(0, pos);
	s2 = substr(pos+1);
	return 2;


}

size_t z_string::split(const char delemiter, z_strlist & list) const
{
	size_t next = 0;
	size_t start=0;
	;
	while(start<size())
	{
		next = find(delemiter,start);
		if (next == string::npos)
			break;
		list.push_back(substr(start,next-start));
		start = next + 1;
		
	} 

	if(start<size())
		list.push_back(substr(start));

	return list.size();
}


int z_string::get_int_val() const
{
	return std::atoi(*this);

}

template <> bool  z_string::get_as_def (bool def) {
	bool val = def;
	try
	{
		val = ("true" == (*this));
	}
	catch (...) {}
	return val;
}
template <> I32  z_string::get_as_def (I32 def) {
	I32 val = def;
	try
	{
		val = std::stol(*this);
	}
	catch (...) {}
	return val;
}
template <> I64  z_string::get_as_def (I64 def) {
	I64 val = def;
	try
	{
		val = std::stoll(*this);
	}
	catch (...) {}
	return val;
}
template <> double  z_string::get_as_def (double def) {
	double val = def;
	try
	{
		val = std::stod(*this);
	}
	catch (...) {}
	return val;
}
template <> U64  z_string::get_as_def (U64 def) {
	U64 val = def;
	try
	{
		val = std::stoull(*this);
	}
	catch (...) {}
	return val;
}
I64 z_string::get_i64_val()
{
	I64 val = 0;
	try
	{
		val = std::stoll(*this);
	}
	catch (...) {}
	return val;
}
double z_string::get_double_val()
{
	double val = 0;
	try
	{
		val = std::stod(*this);
	}
	catch (...) {}
	return val;
}
U64 z_string::get_u64_val()
{
	U64 val = 0;
	try
	{
		val= std::stoull(*this);
	}
	catch (...) {}
	return val;
}
z_string& z_string::copy_escaped(ctext input)
{
	clear();
	size_t i;
	for (i = 0; i<strlen(input); i++)
	{
		char c = input[i];
		switch (c)
		{
		case '\\':
			append("\\\\");
			break;
		case '\n':
			append("\\n");
			break;
		case '\r':
			append("\\r");
			break;
		case '\t':
			append("\\t");
			break;
		case '\'':
			append("\\'");
			break;
		case '\"':
			append("\\\"");
			break;
		default:
			append(1, c);
			break;
		}
	}
	return *this;

}

void z_str_escape(ctext in, std::string& out)
{
	out.clear();
	size_t i;
	out.append("\"");
	for (i = 0; i<strlen(in); i++)
	{
		char c = in[i];
		switch (c)
		{
		case '\\':
			out.append("\\\\");
			break;
		case '\n':
			out.append("\\n");
			break;
		case '\r':
			out.append("\\r");
			break;
		case '\t':
			out.append("\\t");
			break;
		case '\'':
			out.append("\\'");
			break;
		case '\"':
			out.append("\\\"");
			break;
		default:
			out.append(1, c);
			break;
		}
	}
	out.append("\"");
}
void z_str_unescape(std::string& in, std::string& out)
{
	std::string::const_iterator it = in.begin();
	while (it != in.end())
	{
		char c = *it++;
		if (c == '\\' && it != in.end())
		{
			switch (*it++) {
			case '\\': c = '\\'; break;
			case '\"': c = '\"'; break;
			case '\'': c = '\''; break;
			case 'n': c = '\n'; break;
			case 'r': c = '\r'; break;
			case 't': c = '\t'; break;
				// all other escapes
			default:
				// invalid escape sequence - skip it. alternatively you can copy it as is, throw an exception...
				continue;
			}
		}
		out += c;
	}

}
