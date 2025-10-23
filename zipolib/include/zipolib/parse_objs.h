#ifndef z_parse_obj_h
#define z_parse_obj_h
#include "zipolib/z_factory_node.h"

#include "zipolib/parse.h"
#include "zipolib/z_factory_vector.h"

class zp_test_obj : public zp_test_group
{
public:

	virtual z_status test_root(zp_parser* p) override;
	virtual z_status test_run(zp_parser* p) override;
	virtual z_status test_pass(zp_parser* p, ctext match_start, z_status status) override 
	{
		return zs_ok;
	}

};

#define TEST(_X_) template<> void zp_test_obj_t<_X_>::build_test()

template <class CTYPE > class zp_test_obj_t : public zp_test_obj
{

	
	static Tests _tests;
	static Opts _opts;

public:
	virtual Tests& get_tests() {
		if (_tests.size() == 0)
			build_test();
		return _tests;
	}
	virtual Opts& get_opts() {
		return _opts;
	}
	
public:
	zp_test_obj_t()
	{
		_opts.group_type = group_stage;
	}

	virtual void build_test();


	virtual zp_test_group & add_test(zp_test *x)
	{
		_tests.push_back(x);
		return *this;
	}
	virtual z_factory* get_factory() override
	{
		return  GET_FACT(CTYPE);
	}

	virtual z_void_obj* new_object() override
	{
		return  (z_void_obj*) get_factory()->create_void_obj();
	}
	/*
	returns 0 on failure
	*/
	CTYPE* parse(ctext data, z_void_obj* vo = 0)
	{

		CTYPE* obj= (CTYPE*)parse_v(data,vo);
		return obj;


	}


};

template <class TARGET> zp_test& zp_test_group::obj(ctext prop_name )
{
	zp_test_obj_t<TARGET>* test = z_new zp_test_obj_t<TARGET>();
	if (prop_name)
	{
		zf_prop* prop = find_prop(prop_name);
		if (prop)
			test->assign_prop(prop);
		else
		{
			Z_ERROR_MSG(zs_feature_not_found, "prop '%s' not found", prop_name);
		}
	}
	return *test;
}


template <class CTYPE > zp_test_group::Tests zp_test_obj_t<CTYPE>::_tests;
template <class CTYPE > zp_test_group::Opts zp_test_obj_t<CTYPE>::_opts;
/*
class zp_xml_tag_start : public zp_test_group_seq
{
public:
	z_string _name;
	zp_xml_tag_start()
	{
		*this << '<' << ident("name") << '>';
	}
	virtual z_status test_run(zp_parser* p)
	{
		return p->test_char('.');
	};
};

*/



#endif

