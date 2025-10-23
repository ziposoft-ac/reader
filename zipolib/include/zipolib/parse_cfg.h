#ifndef z_parse_cfg_h
#define z_parse_cfg_h
#include "zipolib/parser.h"
#include "zipolib/parse_objs.h"






class zp_cfg_base
{
protected:
	//	zp_test*  _test; // TODO - debug only???
public:
	/*
	zp_cfg_base(zp_test* test)
	{
	_test = test;
	}
	*/
	zp_cfg_base()
	{
		//_test = 0;
	}
	virtual ~zp_cfg_base()
	{

	}
	virtual zp_cfg_base* operator [](size_t i) const
	{
		return 0;
	}
	virtual void get_as_string(z_string& s) {}
	virtual ctext  get_name() { return "generic parse result"; }
	virtual bool save_child_result(zp_cfg_base* child) { return false; }

	virtual void print(int depth)
	{
		z_string s;
		get_as_string(s);
		gz_stdout.indent(depth);
		gz_stdout << get_name() << "=" << s << "\n";
	}
	virtual z_status get(void* var, const z_variable_base* convert)
	{
		return Z_ERROR_NOT_IMPLEMENTED;

	}
};
class zp_cfg_val_str;
class zp_cfg_obj_map;
class zp_cfg_obj_vect;
class zp_cfg_empty_array;
class zp_cfg_obj;


class zp_cfg_val : public zp_cfg_base
{
public:
	zp_cfg_val() : zp_cfg_base()
	{
	}
	virtual ~zp_cfg_val()
	{
	}
	virtual zp_cfg_val_str*  get_string() { return 0; }
	virtual zp_cfg_obj_map*  get_map() { return 0; }
	virtual zp_cfg_obj_vect*  get_vect() { return 0; }
	virtual zp_cfg_empty_array*  get_empty_array() { return 0; }
	virtual zp_cfg_obj*  get_obj() { return 0; }
	virtual ctext  get_name() { return "value"; }

};
class zp_cfg_empty_array : public zp_cfg_val
{
public:
	virtual zp_cfg_empty_array*  get_empty_array() override { return this; };

};

class zp_cfg_val_str : public zp_cfg_val
{
public:
	z_string _val;
	zp_cfg_val_str() : zp_cfg_val()
	{
	}
	virtual ~zp_cfg_val_str()
	{
	}
	virtual ctext  get_name() { return "value"; }
	virtual zp_cfg_val_str*  get_string() override  { return this; } ;

};
class zp_cfg_val_str_list : public zp_cfg_val
{
public:
	zp_cfg_val_str_list() : zp_cfg_val()
	{
	}
	virtual ~zp_cfg_val_str_list()
	{
	}
	virtual ctext  get_name() { return "string list"; }

};
class zp_test_cfg_val_str_list : public zp_test_obj_t<zp_cfg_val_str_list>
{
public:
	virtual z_status test_run(zp_parser* p)
	{ 
		return zp_test_group::test_run(p);
	}
	virtual z_status test_pass(zp_parser* p, ctext match_start, z_status status)
	{
		return zp_test_group::test_pass(p, match_start, status);
	}

};
class zp_cfg_assignment : public zp_cfg_base
{
public:
	z_string _name;
	//z_string _val;
	zp_cfg_val* _val;
	zp_cfg_assignment() : zp_cfg_base()
	{
		_val = 0;
	}
	virtual ~zp_cfg_assignment()
	{
		if (_val)
			delete _val;
	}
	virtual ctext  get_name() { return "assignment"; }

	z_status assign(z_factory* fact, z_void_obj* obj);
};




class zp_cfg_func_call : public zp_cfg_base
{
public:
	zp_cfg_func_call()
	{
	}
	virtual ~zp_cfg_func_call()
	{
	}
	z_string _name;
	z_obj_vector<zp_cfg_assignment> _ass;
};

class zp_cfg_obj : public zp_cfg_val
{
public:
	zp_cfg_obj()
	{
	}
	virtual ~zp_cfg_obj()
	{
	}
	z_string _type;
	z_obj_vector<zp_cfg_assignment> _ass;
	z_obj_vector<zp_cfg_func_call> _funcs;


	z_status assign(z_factory* fact, z_void_obj* obj);

	template<class TYPE>  z_status assign_t(TYPE* obj)
	{
		z_factory* f = GET_FACT(TYPE);
		return assign(f, obj);

	}
	virtual z_status create_obj(z_factory* fact, z_void_obj*& obj) ;
	virtual zp_cfg_obj*  get_obj() override { return this; };

};
class zp_cfg_obj_map_entry : public zp_cfg_val
{
public:
	z_string _name;
	zp_cfg_obj _obj;

	//virtual z_status assign_prop(zf_prop* prop, z_void_obj* obj) override;

};

class zp_cfg_obj_map : public zp_cfg_val
{
public:
	z_obj_vector<zp_cfg_obj_map_entry> _map;
	virtual zp_cfg_obj_map*  get_map() override { return this; };
};

class zp_cfg_obj_vect : public zp_cfg_val
{
public:
	z_obj_vector<zp_cfg_obj> _vect;
	virtual zp_cfg_obj_vect*  get_vect() override { return this; };
};




class zp_cfg_file : public zp_cfg_base
{
public:

};



#endif

