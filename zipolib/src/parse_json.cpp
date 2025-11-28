#include "pch.h"
#include "zipolib/parse_json.h"
#include "zipolib/z_directory.h"

/*

command=testjs
dbname=big_mama
field_name=last_name
search=CORRIVEAU
params[dogs][]=Hot Pie
params[dogs][]=The Jorge
params[dogs][2][name]=monkey balls
params[dogs][2][teeth]=5
params[dogs][]=fdfdfd
params[dogs][4][]=3
params[dogs][4][]=4
params[dogs][4][]=5
params[oh  fuck me]=3
params[bib]=2
req=entrant
action=remove

*/

/*
z_status zp_text_parser::parse_url_params(z_string_map& str_map, z_json_obj& obj)
{
	z_status status = zs_ok;
	for (auto & p : str_map)
	{
		status=parse_url_param(obj,p.first, p.second);

	}
	return status;
}

*/
template <class T> T*  z_json_val::get_as()
{
	Z_THROW_NOT_SUPPORTED;
	return 0;
}
template <> z_json_str* z_json_val::get_as() { return get_as_json_str(); }
template <> z_json_int* z_json_val::get_as() { return get_as_json_int(); }
template <> z_json_float* z_json_val::get_as() { return get_as_json_float(); }
template <> z_json_bool* z_json_val::get_as() { return get_as_json_bool(); }
template <> z_json_array* z_json_val::get_as() { return get_as_json_array(); }
template <> z_json_obj* z_json_val::get_as() {	return get_as_json_obj();}



// Default template specialization just throws
template <class T> z_status z_json_val::get_scalar(T& t)
{
	Z_THROW_NOT_SUPPORTED;
	return zs_not_implemented;
}

template <>  z_status z_json_val::get_scalar(z_string& t)
{
	z_json_str* s = get_as_json_str();
	if (!s)
		return Z_DBG_WARN_RETURN(zs_data_error);
	t = s->_str;
	return zs_ok;
}

template <>  z_status z_json_val::get_scalar(int64_t& t)
{
	z_json_int* s = get_as_json_int();
	if (!s) return Z_DBG_WARN_RETURN(zs_data_error);
	t = s->_val;
	return zs_ok;
}
template <>  z_status z_json_val::get_scalar(U32& t)
{
	z_json_int* s = get_as_json_int();
	if (!s) return Z_DBG_WARN_RETURN(zs_data_error);
	t = s->_val;
	return zs_ok;
}



template <>  z_status z_json_val::get_scalar(bool& t)
{
	z_json_bool* s = get_as_json_bool();
	if (!s) return Z_DBG_WARN_RETURN(zs_data_error);
	t= s->_val;
	return zs_ok;
}

template <>  z_status z_json_val::get_scalar(double& t)
{
	z_json_float* s = get_as_json_float();
	if (!s) return Z_DBG_WARN_RETURN(zs_data_error);
	t = s->_val;
	return zs_ok;
}


z_status zp_text_parser::parse_url_param(z_json_obj& obj,const z_string& key_line, const z_string& value)
{
	enum kv_type
	{
		kv_simple,
		kv_array,
		kv_obj
	};

	

	//params[dogs][2][name] = monkey balls
	z_json_container* pcont=&obj;
	set_source(key_line);
	z_string key;
	if (test_any_identifier())
		return Z_ERROR(zs_parse_error);
	get_match(key);//params
		
	while (1)
	{
		z_string next_key;
		kv_type type = kv_simple;
		z_json_container* next_cont =dynamic_cast<z_json_container*>( pcont->get_val(key));
		if (test_char('[') == zs_ok)
		{

			if (test_char(']') == zs_ok)
			{
				type = kv_array;
				//add new to array
				//this means the container we are point at 
				//must be an array.
			}
			else
			{
				if (test_integer() == zs_ok)
				{
					type = kv_array;
					//this means the container we are point at 
					//must be an array.
				}
				else
				{
					//must be an object
					if (test_any_identifier())//dogs
						return Z_ERROR(zs_parse_error);
					type = kv_obj;
				}
				get_match(next_key);//dogs

				// TODO - support spaces in keys
				if (test_char(']'))
					return Z_ERROR_MSG(zs_parse_error, "expecting ']' (spaces not supported in keys yet) ");
			}
			if (!next_cont)
			{
				if (type == kv_array)
					next_cont = z_new z_json_array();
				if (type == kv_obj)
					next_cont = z_new z_json_obj();
				if (!pcont->add_val(key, next_cont))
				{
					delete next_cont;
					return Z_ERROR_MSG(zs_parse_error, "bad key trying to add sub container");

				}
			}
			pcont = next_cont;
			key = next_key;
		}
		else
			break;

	}//while (1)
	//should not exist

	// the values are just unquoted strings

	//TODO - handle duplicate keys
	z_json_str* js = z_new z_json_str(value);
	if (!pcont->add_val(key, js))
	{
		//wasnt added
		delete js;
	}



	return zs_ok;


}

z_status zp_text_parser::parse_json_file(ctext filename, z_json_val*& json_val)
{
	size_t length = 0;
	char* buffer = 0;
	z_status status = z_file_open_and_read(filename, &length, &buffer);
	if (status)
		Z_THROW_MSG(zs_could_not_open_file, "Could not open file:%s", filename);
	status = parse_json(buffer, length, json_val);


	return status;



}

z_json_obj zp_text_parser::parseJsonObj(ctext input, size_t len)
{
    z_json_obj obj;
    set_source(input,len);

    z_status status;
    do {
        status = test_char('{');
        if (status)
        	break;
        status=parse_json_obj_contents(obj);
        if (status)
        {
            obj.clear();
            break;
        }
        status = test_char('}');
        if (status) obj.clear();

    }while(0);

    return obj;
}
z_status zp_text_parser::parse_json(ctext json, size_t len, z_json_val*& valout)
{

	set_source(json, len);
	z_json_val* v = parse_json_val();
	if (v)
	{
		valout = v;
		return zs_ok;

	}

	valout = 0;
	return zs_not_found;
}
z_json_null* zp_text_parser::parse_json_null()
{
	if (test_string("null",4))
		return 0;
	z_json_null* jb = z_new z_json_null();
	return jb;

}
z_json_bool* zp_text_parser::parse_json_bool(ctext str, bool val)
{
	if (test_string(str))
		return 0;
	z_json_bool* jb = z_new z_json_bool();
	jb->_val = val;
	return jb;

}
z_status zp_text_parser::parse_json_obj(const z_string& input, z_json_obj& obj)
{

	set_source(input);
	z_status status = test_char('{');
	if (status)
		Z_THROW("Expected '{'");
	status=parse_json_obj_contents(obj);
	if (status)
		return status;// 
	status = test_char('}');
	if (status)
		Z_THROW("Expected '}'");
	return status;



}


z_status zp_text_parser::parse_json_obj_contents(z_json_obj& obj)
{

	while (1)
	{
		skip_ws();
		z_string key;
		//all keys should be quoted
		if (test_code_string())
		{
			//but we allow non- quoted keys
			if (test_any_identifier())
				break;
		}

		z_string s;
		get_match(s);
		z_str_unescape(s, key);
		skip_ws();
		if (test_char(':'))
			break;
		skip_ws();
		z_json_val* val = parse_json_val();
		if (val)
		{
			obj._keys.add(key, val);
		}
		else
		{
			//throw error
			return Z_ERROR_MSG(zs_parse_error, "error parsing value");
		}
		if (test_char(','))
			break;
	}
	skip_ws();
	

	return zs_ok;
}

z_json_val* zp_text_parser::parse_json_val()
{
	z_status status = zs_ok;
	skip_ws();
	if (!test_char('{'))
	{
		//its an object
		z_json_obj* obj = z_new z_json_obj();

		status=parse_json_obj_contents(*obj);
		if (status==zs_ok)
			if (!test_char('}'))
				return obj;
		delete obj;
		//throw error
		return 0;
	}
	if (!test_char('['))
	{
		z_json_array* arr = z_new z_json_array();

		while (1)
		{
			z_json_val* val = parse_json_val();
			if (val)
			{
				arr->_array.add(val);
			}
			else
				break;
			if (test_char(','))
				break;
		}
		skip_ws();

		if (!test_char(']'))
			return arr;
		//throw error
		delete arr;
		return 0;
	}


	if ((test_code_string()==zs_ok)|| (test_single_quoted_string() == zs_ok))
	{
		z_json_str* js = z_new z_json_str();
		z_string s;
		get_match(s);
		z_str_unescape(s, js->_str);
		return js;
	}
	z_json_null* jn;
	if (jn= parse_json_null())
		return jn;

	z_json_bool* jb;
	if (jb= parse_json_bool("FALSE",false))
		return jb;
	if (jb = parse_json_bool("false", false))
		return jb;
	if (jb = parse_json_bool("TRUE", true))
		return jb;
	if (jb = parse_json_bool("true", true))
		return jb;
	if (!test_floating_point())
	{
		z_json_float* js = z_new z_json_float();
		z_string s;
		get_match(s);
		//todo
		
		js->_val=s.get_double_val();
		return js;
	}
	if (!test_integer())
	{
		z_json_int* js = z_new z_json_int();
		z_string s;
		get_match(s);
		js->_val = s.get_i64_val();
		return js;
	}

	// HACK



	return 0;
}
I64 z_json_obj::get_int(ctext key,I64 def)
{
    I64 val=def;
    get_int_val(key,val);
    return val;
}
bool z_json_obj::get_int_val(ctext key, I64 &i)
{
	z_json_val* jv = get_val(key);
	if (!jv)
		return false;
	if (jv->get_scalar(i) == zs_ok)
		return true;
	z_string str;
	if (jv->get_scalar(str) != zs_ok)
		return false;
	i = str.get_i64_val();
	return true;
}
z_string z_json_obj::get_str_def(ctext key,ctext def)
{
    z_string s;
	z_json_str* ji = get_val_t<z_json_str>(key);
	if (ji)
	{
		s = ji->_str;
	}
	return s;

}
bool z_json_obj::get_bool(ctext key, bool& val, bool def)
{
	z_json_bool* ji = get_val_t<z_json_bool>(key);
	if (ji)
	{
		val = ji->_val;
		return true;
	}
	val = def;
	return false;

}

z_json_val* load_json_file(ctext file_name)
{
	zp_text_parser parser;
	size_t length = 0;
	char* buffer = 0;
	z_status status = z_file_open_and_read(file_name, &length, &buffer);
	if (status)
		Z_THROW_MSG(zs_could_not_open_file, "Could not open file:%s", file_name);
	z_json_val* val = 0;
	status=parser.parse_json(buffer, length, val);
	if (status)
		Z_THROW_MSG(status, "Error parsing json file:%s", file_name);

	return val;
}
