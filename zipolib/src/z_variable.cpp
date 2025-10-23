/*________________________________________________________________________

 z_variable_

________________________________________________________________________*/
#include "pch.h"

#include <iostream>
#include <sstream>
#include "zipolib/z_variable.h"
#include "zipolib/z_parse_text.h"
#include <cmath>
z_status from_string(const z_string &s, z_strlist& type)
{

	return zs_ok;

}

#define ZC template <> z_status z_variable
#define RECAST(_TYPE_,_NAME_) _TYPE_& _NAME_= *reinterpret_cast<_TYPE_*>(v);


/*________________________________________________________________________

z_variable<TYPE> defaults
________________________________________________________________________*/
template <class V> z_status z_variable<V>::load_from_parser(zp_text_parser &parser, void* v) const
{
	z_status status = parser.test_file_path();
	if (status)
		// TODO - have variable specific error message fn to return what is expected
		// like "expecting true/false"
		return zs_parse_error;
	if (!v)
		return Z_ERROR(zs_bad_parameter);

	z_string s;
	parser.get_match(s);
	return set_from_string(s, v);
}
template <class V> z_status z_variable<V>::set_from_string(z_string& s, void* v) const
{
	if (!v)
		return Z_ERROR(zs_bad_parameter);

	RECAST(V, var);
	std::stringstream instream(std::ios::in);
	instream.str(s);
	instream >> var;
	return zs_ok;
}
template <class V> z_status z_variable<V>::get_as_string(z_string& s, void* v) const
{
	if (!v)
		return Z_ERROR(zs_bad_parameter);
	RECAST(V, var);
	s = var;
	return zs_ok;
}

template <class V> z_status z_variable<V>::set_from_int(I64& intVal, void* v) const
{
	if (!v)
		return Z_ERROR(zs_bad_parameter);

	RECAST(V, var);
	var = (V)intVal;
	return zs_ok;
}
template <class V> z_status z_variable<V>::set_from_float(double& val, void* v) const
{
	if (!v)
		return Z_ERROR(zs_bad_parameter);
	RECAST(V, var);
	var = static_cast<V>(val);
	return zs_ok;
}
//template <class V> z_status z_variable<V>::set_from_parse_vector(zp_result_vector* cc, void* v) const { return Z_ERROR_NOT_IMPLEMENTED; };

/*________________________________________________________________________

int
________________________________________________________________________*/



/*________________________________________________________________________

ctext
________________________________________________________________________*/
ZC<ctext>::set_from_int(I64& intVal, void* v) const
{
	return Z_ERROR(zs_invalid_conversion);
}
ZC<ctext>::set_from_float(double& intVal, void* v) const
{
	return Z_ERROR(zs_invalid_conversion);
}

ZC<ctext>::set_from_string(z_string& s, void* v) const
{
	return Z_ERROR(zs_invalid_conversion);
}

ZC<ctext>::get_as_string(z_string& s, void* v) const
{
	RECAST(ctext, var);
	s = '\"';
	s << var << '\"';
	return zs_ok;
}

ZC<ctext>::load_from_parser(zp_text_parser& parser, void* v) const
{
	return Z_ERROR(zs_invalid_conversion);
}


/*________________________________________________________________________

z_string
________________________________________________________________________*/
ZC<std::string>::set_from_int(I64& intVal, void* v) const
{
	return Z_ERROR(zs_invalid_conversion);
}
ZC<std::string>::set_from_float(double& intVal, void* v) const
{
	return Z_ERROR(zs_invalid_conversion);
}
ZC<z_string>::set_from_float(double& val, void* v) const
{
	if (!v)
		return Z_ERROR(zs_bad_parameter);

	RECAST(z_string, var);
	var = std::to_string(val);
	return zs_ok;
}
ZC<z_string>::set_from_int(I64& intVal, void* v) const
{
	if (!v)
		return Z_ERROR(zs_bad_parameter);

	RECAST(z_string, var);
	var = intVal;
	return zs_ok;
}
ZC<z_string>::set_from_string(z_string& s, void* v) const
{
	RECAST(z_string, var);
	var = s;
	return zs_ok;
}

ZC<z_string>::get_as_string(z_string& s, void* v) const
{
	RECAST(z_string, var);
	s = '\"';
	s << var << '\"';
	return zs_ok;
}

ZC<z_string>::load_from_parser(zp_text_parser& parser, void* v) const
{
	// If v==0 advance the parser only
	RECAST(z_string, var);
	z_status status = parser.test_code_string();
	if (status == zs_no_match)
	{
		status = parser.test_single_quoted_string();
		if (status == zs_no_match)
		{
			//this is a bad shortcut.
			// allow setting strings with quotes if it is a filepath

		 // What was this for? To allow unquoted strings? hmmmm
		 //status = parser.test_file_path();
			var = "";
		}
	}
	if (status)
		return status;
	if (!v)
		return Z_ERROR(zs_bad_parameter);

	parser.get_match(var);
	return zs_ok;
}


/*________________________________________________________________________

char
________________________________________________________________________*/
ZC<char>::load_from_parser(zp_text_parser& parser, void* v) const
{
	z_status status = parser.test_single_quoted_string();
	if (status)
		// TODO - have variable specific error message fn to return what is expected
		// like "expecting true/false"
		return zs_parse_error;
	if (!v)
		return Z_ERROR(zs_bad_parameter);

	z_string s;
	parser.get_match(s);
	return set_from_string(s, v);
}


/*________________________________________________________________________

int
________________________________________________________________________*/
ZC<int>::set_from_float(double& val, void* v) const
{
	if (!v)
		return Z_ERROR(zs_bad_parameter);
	RECAST(int, var);
	var = std::lround(val);
	return zs_ok;
}

ZC<int>::load_from_parser(zp_text_parser& parser, void* v) const
{
	z_status status;
	RECAST(int, var);
	z_string s;

	if (parser.test_hex_integer() == zs_matched)
	{
		parser.get_match(s);
		var = std::stoul(s, nullptr, 16);
		return zs_ok;
	}
	status = parser.test_integer();
	if (status)
		// TODO - have variable specific error message fn to return what is expected
		// like "expecting true/false"
		return zs_parse_error;
	if (!v)
		return Z_ERROR(zs_bad_parameter);



	parser.get_match(s);

	var = std::stoul(s, nullptr,  10);



	return zs_ok;
}

/*________________________________________________________________________

 BOOL
________________________________________________________________________*/

ZC<bool>::set_from_string(z_string& s, void* v) const
{
	if (!v)
		return Z_ERROR(zs_bad_parameter);
	RECAST(bool, var);
	var = (s == "true") || (s == "on")|| (s == "1");

	return zs_ok;
}
ZC<bool>::set_from_int(I64& intVal, void* v) const
{
	if (!v)
		return Z_ERROR(zs_bad_parameter);
	RECAST(bool, var);
	var = (intVal != 0);
	return zs_ok;
}
ZC<bool>::set_from_float(double& val, void* v) const
{
	if (!v)
		return Z_ERROR(zs_bad_parameter);
	RECAST(bool, var);
	var = (val != 0);
	return zs_ok;
}
/*________________________________________________________________________

z_variable<z_strlist>
________________________________________________________________________*/
ZC<z_strlist>::load_from_parser(zp_text_parser &parser, void* v) const
{
	RECAST(z_strlist, list);

	if (!v) //TODO! This will not advance the parser
		return Z_ERROR(zs_bad_parameter);


	list.clear(); // TODO empty it?
	z_status status;
	parser.skip_ws();
	status = parser.test_string("str"); //throw away
	status = parser.test_char('[');

	if (status)
		return Z_ERROR_MSG(status, "Expected '[' ");
	z_string s;

	while (status == zs_ok)
	{
		parser.skip_ws();
		status = parser.test_code_string();
		if (status)
			status = parser.test_single_quoted_string();
		if (status)
			break;

		parser.get_match(s);

		z_string unesc;
		z_str_unescape(s, unesc);
		list << unesc;
		parser.skip_ws();

		status = parser.test_char(',');
		if (status)
			break;

	}
	parser.skip_ws();
	status = parser.test_char(']');
	if (status)
		return Z_ERROR_MSG(status, "Expected ']' ");

	return zs_ok;
}
ZC<z_strlist>::set_from_string(z_string& s, void* v) const
{
	zp_text_parser p(s);
	load_from_parser(p, v);
	return zs_ok;
}
ZC<z_strlist>::get_as_string(z_string& s, void* v) const
{
	RECAST(z_strlist, list);
	s = "";
	s << "[";
	list.get_as_string(s);
	s << ']';

	return zs_ok;
}



ZC<z_strlist>::set_from_int(I64& intVal, void* v) const
{
	return Z_ERROR(zs_invalid_conversion);
}
ZC<z_strlist>::set_from_float(double& intVal, void* v) const
{
	return Z_ERROR(zs_invalid_conversion);
}


/*________________________________________________________________________

z_time
________________________________________________________________________*/
ZC<z_time>::set_from_int(I64& intVal, void* v) const
{
	RECAST(z_time, t);
	t.set_ptime_ms(intVal);
	return zs_ok;
}
ZC<z_time>::set_from_float(double& intVal, void* v) const
{
	return Z_ERROR(zs_invalid_conversion);
}

ZC<z_time>::set_from_string(z_string& s, void* v) const
{
	RECAST(z_time, t);
	t.set_from_iso_string(s);
	if(t.is_not_a_date_time())
		return Z_ERROR(zs_invalid_conversion);
	return zs_ok;
}

ZC<z_time>::get_as_string(z_string& s, void* v) const
{
	RECAST(z_time, t);
	s = '\"';
	s << t.to_iso_string() << '\"';
	return zs_ok;
}

ZC<z_time>::load_from_parser(zp_text_parser& parser, void* v) const
{
	z_status status;
	z_string s;
	status = parser.test_code_string();
	if (status)
		return Z_ERROR(zs_invalid_conversion);
	parser.get_match(s);
	return set_from_string(s,v);
}



template class z_variable<z_time>;
template class z_variable<z_string>;
template class z_variable<std::string>;
template class z_variable<unsigned long long>;
template class z_variable<long long>;
//template class z_variable<float>;
template class z_variable<double>;
template class z_variable<int>;
template class z_variable<char>;
template class z_variable<ctext>;
template class z_variable<unsigned long>;
template class z_variable<unsigned int>;
template class z_variable<bool>;
//template class z_variable<z_strlist>;
