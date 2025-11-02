#ifndef parse_json_h
#define parse_json_h
#include "zipolib/z_parse_text.h"
#include "zipolib/z_stream_json.h"


class z_json_str;
class z_json_array;
class z_json_obj;
class z_json_int;
class z_json_float;
class z_json_val;
class z_json_bool;
class z_json_container;
/*
class z_json_parser : zp_text_parser
{
	z_json_bool* parse_json_bool(ctext str, bool val);
	z_json_val* parse_json_val();
public:

	z_status parse_json(ctext json, size_t len, z_json_val*& val);
	z_status parse_url_params(z_string_map& str_map, z_json_obj& obj);
	z_status parse_url_param(z_json_obj& obj,const z_string& key_line, const z_string& value);

};

*/
class z_json_val 
{
public:
	z_json_val() 
	{
	}
	virtual ~z_json_val()
	{
	}
	template <class T > z_status get_vector(std::vector<T> vect);

	template <class T > z_status get_scalar(T& t);
	template <class T > T*  get_as();
	virtual z_json_str* get_as_json_str() { return 0; }
	virtual z_json_int* get_as_json_int() { return 0; }
	virtual z_json_bool*  get_as_json_bool() { return 0; }
	virtual z_json_array*  get_as_json_array() { return 0; }
	virtual z_json_float* get_as_json_float() { return 0; }
	virtual z_json_obj*  get_as_json_obj() { return 0; }
	virtual void get_as_string(z_string& s) {}
	virtual ctext  get_name() { return "value"; }
	virtual void print(z_json_stream &s)
	{
		
	}
};

class z_json_str : public z_json_val
{
public:
	z_json_str(ctext val)
	{
		_str = val;
	}
	z_json_str()
	{
	}
	virtual ~z_json_str()
	{
	}
	z_string _str;
	virtual z_json_str*  get_as_json_str() override { return this; };
	virtual void print(z_json_stream &s)
	{
		s % _str;
	}

	virtual void get_as_string(z_string& s) override { s = _str; }

};
class z_json_int : public z_json_val
{
public:
	int64_t _val;
	virtual z_json_int* get_as_json_int() override { return this; };
	virtual void print(z_json_stream &s)
	{
		s % _val;
	}
};
class z_json_bool : public z_json_val
{
public:
	bool _val;
	virtual void print(z_json_stream &s)
	{
		s % _val;
	}
};
class z_json_float : public z_json_val
{
public:
	virtual z_json_float* get_as_json_float() { return this; }

	double _val;
	virtual void print(z_json_stream &s)
	{
		s % _val;
	}
};

class z_json_container : public z_json_val
{
public:

	virtual bool add_val(ctext key, z_json_val* val) = 0;
	virtual bool add_val(ctext key, ctext val) {
		z_json_str* s = z_new z_json_str(val);

		return add_val(key,s);
	}
	virtual z_json_val* get_val(ctext key) = 0;
	virtual void clear() = 0;

};

class z_json_obj : public z_json_container
{
	z_json_obj(const z_json_obj&)
	{
	}
public:
	z_json_obj()
	{
	}
	virtual ~z_json_obj()
	{
	}
	z_json_obj( z_json_obj&& obj)=default;

	z_string _type;
	z_obj_map<z_json_val> _keys;
	virtual z_json_obj*  get_as_json_obj() override { return this; };
	virtual z_json_obj*  get_child(ctext key) { return get_val_t<z_json_obj>(key); };

	

	virtual void clear()
	{
		_keys.delete_all();
	}

	virtual bool add_val(ctext key, z_json_val* val) {

		z_json_val* exist = _keys.getobj(key);
		if (exist)
		{
			_keys.remove(key);
			delete exist;
		}
		return _keys.add(key, val);
	}
	virtual z_json_val* get_val(ctext key)
	{
		return _keys.getobj(key);
	}
	virtual void print( z_json_stream &stream)
	{
		bool pp = false;

		if (_keys.size() == 0)
			pp = stream.set_pretty_print(false);
		stream.obj_start();
		for (auto p : _keys)
		{
			stream.key(p.first);
			p.second->print(stream);
		}
		stream.obj_end();
		if (_keys.size() == 0)
			stream.set_pretty_print(pp);
	}

	I64 get_int(ctext key,I64 def=0);
	bool get_int_val(ctext key, I64 &i);
	bool get_str(ctext key,z_string& s, ctext def);
	z_string get_str_def(ctext key,ctext def);
	bool get_bool(ctext key, bool& s, bool def);
	template <class VALTYPE> VALTYPE* get_val_t(ctext key)
	{
		z_json_val* val = _keys.getobj(key);
		if (!val) return 0;
		return val->get_as<VALTYPE>();

	}
};


class z_json_array : public z_json_container
{
public:
	z_json_array()
	{
	}
	virtual ~z_json_array()
	{
	}
	z_obj_vector<z_json_val> _array;
	size_t get_size() { return _array.size(); }
	z_json_val* get(size_t i) { 
		if (i >= get_size())
		{
			Z_THROW_MSG(zs_data_error, "json value out of array range");
		}
		return _array[i]; 
	}

	template <class T > T get_scalar(size_t i)
	{
		z_json_val* jv = get(i);
		T var;
		if(jv->get_scalar<T>(var)!=zs_ok)
			Z_THROW_MSG(zs_data_error, "invalid json scalar type");
		

		return var;
	}

	virtual void clear()
	{
		_array.destroy();
	}
	virtual z_json_array*  get_as_json_array() override { return this; };
	virtual bool add_val(ctext key, z_json_val* val) {
		if (!key)
			return false;
		if (strlen(key))
		{
			//only add a new item if key is top
			size_t i = atoi(key);
			if (i != _array.size())
				return false;
		}
		_array.push_back(val);
		return true;
	}
	virtual z_json_val* get_val(ctext key)
	{
		size_t i = atoi(key);

		return _array.get(i);
	}

	virtual void print( z_json_stream &stream)
	{
		bool pp = false;
		if (_array.size() == 0)
			pp = stream.set_pretty_print(false);

		stream.array_start();
		for (z_json_val* entry : _array)
		{
			entry->print(stream);

		}
		stream.array_end();
		if (_array.size() == 0)
		stream.set_pretty_print(pp);

	}

};


template <class T > z_status z_json_val::get_vector(std::vector<T> vect)
{
	z_json_array* arr = get_as_json_array();
	if (!arr)
	{
		Z_THROW_MSG(zs_data_error,"json value is not an array");
	}
	size_t i;
	vect.clear();
	for (i = 0; i < arr->get_size(); i++)
	{
		T scalar;
		z_json_val* val = arr->get(i);
		z_status status = val->get_scalar(scalar);
		if (status)
		{
			Z_THROW_MSG(status, "bad json array value");
		}
		vect.push_back(scalar);
	}
	return zs_ok;



}
z_json_val* load_json_file(ctext file_name);

#endif

