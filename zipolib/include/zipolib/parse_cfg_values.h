#ifndef z_parse_cfg_vals_h
#define z_parse_cfg_vals_h
#include "zipolib/parser.h"
#include "zipolib/parse_cfg.h"

class zp_cfg_value : public zp_cfg_base
{
public:
	zp_cfg_value() { }
	virtual ~zp_cfg_value() {
	}
	virtual ctext  get_name() { return "unknown"; }

};

class zp_cfg_string : public zp_cfg_value
{
public:
	z_string _match;
	zp_cfg_string() {}
	virtual void get_as_string(z_string& s) { s = _match; }
	virtual z_status get(void* var, const z_variable_base* convert)
	{
		return Z_ERROR_NOT_IMPLEMENTED;
	}
	virtual ~zp_cfg_string() {}
	virtual ctext  get_name() { return "string"; }

};

class zp_cfg_path : public zp_cfg_string
{
public:
	zp_cfg_path() : zp_cfg_string()
	{

	}
	virtual ctext  get_name() { return "path"; }

};


template <class CTYPE, z_status(zp_text_parser::*PARSE_FUNC)()> class zp_test_obj_simple_t : public zp_test
{
public:

	zp_test_obj_simple_t()
	{

	}
	CTYPE* parse(ctext data, z_void_obj* vo = 0)
	{

		return (CTYPE*)parse_v(data,vo);


	}


	virtual z_void_obj* new_object() {
		z_factory* f = GET_FACT(CTYPE);
		z_void_obj* me=(z_void_obj*)f->create_void_obj();
		return me;
	}
	virtual z_status test_run(zp_parser* p)
	{

		zf_prop* prop = get_prop();

		z_void_obj* obj_parent = 0;
		z_void_obj* obj_child = 0;


		if (prop)
		{
			obj_parent = p->get_current_obj(false);
			if (obj_parent)
				obj_child = (z_void_obj*)prop->obj_ptr_get(obj_parent);


			p->context_push(this, obj_child);
		}

		z_status status = (p->*PARSE_FUNC)();
	
		if (status == zs_ok)
		{
			z_void_obj* obj_this = 0;
			z_factory* f = GET_FACT(CTYPE);
			ZT("%s passed", get_name());
			z_string s;
			zf_prop* sub_prop = dynamic_cast<zf_prop*>(f->get_feature_idx(0));
			p->get_match(s);
			obj_this = p->get_current_obj(true);
			sub_prop->set_from_string(s, obj_this);
		}
		else

		{
			ZT("%s failed", get_name());
		}

		if (prop)
		{
			if (status) //failed
			{

				p->unwind_obj();
			}
			else
			{
				obj_parent = p->get_parent_obj(true);
				obj_child = p->get_current_obj(true);
				prop->obj_ptr_set(obj_parent, obj_child);
			}
			p->context_pop();
		}
		else
		{
			if (status)
			{
				p->unwind_obj();
			}
		}


		return status;



	}

};
class zp_cfg_quoted_string : public zp_cfg_string
{
public:
	zp_cfg_quoted_string() : zp_cfg_string()
	{

	}
	virtual ctext  get_name() { return "quoted string"; }

};
class zp_test_cfg_quoted_string : public zp_test_obj_t<zp_cfg_quoted_string>
{
public:
	virtual void build_test();

};

class zp_cfg_single_quoted_string : public zp_cfg_string
{
public:
	zp_cfg_single_quoted_string() : zp_cfg_string()
	{
	}
	virtual ctext  get_name() { return "single quoted string"; }

};

typedef zp_test_obj_simple_t<zp_cfg_single_quoted_string, &zp_parser::test_single_quoted_string> zp_test_cfg_single_quoted_string;
class zp_cfg_int : public zp_cfg_value
{
public:
	I64 _val;
	zp_cfg_int()
	{
		_val = 0;
	}
	virtual ~zp_cfg_int()
	{
	}
	virtual z_status get(void* var, const z_variable_base* convert) {
		return Z_ERROR_NOT_IMPLEMENTED;

	}
	virtual void get_as_string(z_string& s) { s = _val; }

	virtual ctext  get_name() { return "integer"; }

};

typedef zp_test_obj_simple_t<zp_cfg_int, &zp_parser::test_integer> zp_test_cfg_int;

class zp_cfg_float : public zp_cfg_value
{
public:
	double _val;
	zp_cfg_float()
	{
		_val = 0;
	}
	virtual z_status get(void* var, const z_variable_base* convert) {
		return Z_ERROR_NOT_IMPLEMENTED;

	}
	virtual void get_as_string(z_string& s) { s = _val; }
	virtual ctext  get_name() { return "floating point number"; }

};
typedef zp_test_obj_simple_t<zp_cfg_float, &zp_parser::test_floating_point> zp_test_cfg_float;


class zp_cfg_vector : public zp_cfg_value
{
protected:

	z_obj_vector<zp_cfg_base, true> _results;
public:
	zp_cfg_vector()
	{
	}
	virtual ctext  get_name() { return "parse vector"; }
	virtual z_status get(void* var, const z_variable_base* convert) {
		return Z_ERROR_NOT_IMPLEMENTED;

	}
	virtual ~zp_cfg_vector()
	{

	}
	template<class TYPE>  TYPE* get_result_t(size_t index)
	{
		if (index < _results.size())
		{
			TYPE* pobj = dynamic_cast<TYPE*>(_results.get(index));
			if (pobj)
			{
				return pobj;
			}
		}
		return 0;
	}
	zp_cfg_base* get_next_result(zp_cfg_base*& pobj, size_t& index)
	{
		pobj = 0;
		if (index < _results.size())
		{
			pobj = _results.get(index);
			index++;
		}

		return pobj;
	}
	template<class TYPE>  TYPE* get_next_result_t(TYPE*& pobj, size_t& index)
	{
		while (index < _results.size())
		{
			pobj = dynamic_cast<TYPE*>(_results.get(index));
			index++;
			if (pobj)
			{
				return pobj;
			}
		}
		pobj = 0;
		return 0;
	}
	size_t get_count() { return _results.size(); }
	virtual zp_cfg_base* operator [](size_t i) const
	{
		return _results.get(i);
	}
	virtual bool save_child_result(zp_cfg_base* result)
	{
		const std::type_info & info = typeid(*result);
		ZT("adding %s", info.name());
		_results.add(result);
		return true;

	}
	virtual void print(int depth);
};

class zp_cfg_assignment2 : public zp_cfg_base
{
public:
	z_string _name;
	zp_cfg_value* _val;
	zp_cfg_assignment2() : zp_cfg_base()
	{
		_val = 0;
		_name = "monkey";
	}
	virtual ~zp_cfg_assignment2()
	{
		if (_val)
			delete _val;
	}
	virtual ctext  get_name() { return "assignment"; }
	virtual void print(int depth)
	{
		gz_stdout << _name << "=" << (size_t)_val << "\n";
	}

	z_status get_name(z_string &name)
	{
		return Z_ERROR(zs_not_found);
	}
	zp_cfg_value* get_value()
	{
		return _val;


	}
};



#endif

