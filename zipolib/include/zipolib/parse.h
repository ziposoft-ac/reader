#ifndef z_parse_h
#define z_parse_h
#include "zipolib/zipolib.h"
#include "zipolib/z_terminal.h"
#include "zipolib/z_parse_text.h"
class zp_parser;
class zp_result_obj_contents;
class z_stream_dummy : public z_stream
{

	z_status write(const char* data, size_t len)
	{
		return zs_ok;
	}

};

union zp_flags {
	struct {
		U32 multi : 1;
		U32 required : 1;
		U32 commited : 1; //causes parent stage to return error if no match
		U32 consume_all : 1;
		/*
		U32 ignore_ws : 1;
		U32 parent_data : 1;
		U32 multi : 1;
		U32 required : 1;
		U32 parent_name : 1; //???
		U32 ignore_nl : 1; //???
		U32 create_default : 1;
		U32 this_obj : 1;

		U32 group_single : 1;
		U32 random : 1;
		//U32 nested_group:1;
		*/
	
	};
	/*
	struct {
		U32 group_or : 1;
		U32 group_and : 1;
		U32 group_stage : 1;
	};
	*/
	U32 as_u32;
};


/*_______________________________________________________________________________________________
						SINGLE TESTS
*/
class zp_test_group_or;
class zf_prop;
class z_factory;
class zf_node;
class zp_test_obj;

class zp_test
{
	zf_prop* _prop;
protected:
	zp_flags _flags;
public:
	enum
	{
		group_stage,
		group_or,
		group_and,
	};
	ctext _name; // TODO make private

	zp_flags& flags() {
		return _flags;
	}
	virtual zp_test & iws() { return *this; };
	virtual zp_test & debug() { return *this; };
	zp_test()
	{
		_flags.as_u32 = 0;
		_flags.required = 1;
		_name = 0;
		_prop = 0;
	}
	zp_test(ctext  name)
	{
		_prop = 0;
		_name = name;
	}

	virtual zf_prop* get_prop() {
		return _prop;
	}

	virtual ctext get_name();

	virtual void report_expecting(z_string &s) {}
	virtual ~zp_test() {};
	virtual z_status test_run(zp_parser* p) { return zs_not_implemented; };
	virtual z_status test_root(zp_parser* p) { return test_run(p); };
	virtual z_status test_pass(zp_parser* p);
	virtual zp_test& assign_prop(zf_prop* prop);

	virtual void delete_children() {};
	//virtual z_status add_child_result(zp_test* child, zp_result** context, zp_result* child_result);

	virtual z_void_obj* new_object() {
		return  0;
	}
	virtual z_factory* get_factory() 
	{
		return  0;
	}

	
	z_void_obj* parse_v(ctext data, z_void_obj* vo = 0);

	virtual zp_test & operator << (zf_prop *prop)
	{
		assign_prop(prop);
		return *this;
	};
};

class zp_test_str : public zp_test
{
public:
	z_string  _s;
	zp_test_str(ctext s)
	{
		_s = s;
	}
	virtual z_status test_run(zp_parser* p);
};

class zp_test_char : public zp_test
{
public:
	char _c;
	zp_test_char(char x)
	{
		_c = x;
	}
	virtual z_status test_run(zp_parser* p);
};
class zp_test_wsp : public zp_test
{

public:
	bool _save;
	zp_test_wsp()
	{
		_save = false;
	}

	virtual z_status test_run(zp_parser* p);
};


class zp_test_group : public zp_test
{
public:
	union Opts {
		struct {
			U32 group_type : 4;
			U32 ignore_ws : 1;
			U32 debug : 1;
		};
		U32 as_u32;
	};

	virtual zp_test & iws()
	{
		get_opts().ignore_ws = 1;
		return *this;
	};
	virtual zp_test & debug()
	{
		get_opts().debug = 1;
		return *this;
	};
	typedef  	z_obj_vector<zp_test, true>  Tests;
	virtual Tests& get_tests() = 0;
	virtual Opts& get_opts() = 0;

	virtual zp_test_group & add_test(zp_test *x) = 0;
	zp_test_group() {
	
	}

	virtual ~zp_test_group()
	{

	};
	virtual z_status test_run(zp_parser* p);
	virtual z_status test_pass(zp_parser* p, ctext match_start, z_status status);
	//virtual z_status test_run(zp_parser* p) { return Z_ERROR_NOT_IMPLEMENTED; };

	zp_test_group& operator()(zf_prop* prop) {
		assign_prop(prop);
		return *this;
	}

	virtual zp_test_group & operator << (zp_test &x)
	{
		return add_test(&x);
	};
	virtual zp_test_group & operator << (zp_test *x)
	{
		return add_test(x);
	};
	virtual zp_test_group & operator << (char c)
	{
		zp_test_char* x = z_new zp_test_char(c);
		return *this << x;
	};
	zp_test& str(ctext c)
	{
		zp_test_str* test = z_new zp_test_str(c);
		return *test;
	}
	zp_test& chr(char c)
	{
		zp_test_char* test = z_new zp_test_char(c);
		return *test;
	}
	zp_test& zero_or_more(zp_test& test)
	{
		test.flags().multi = 1;
		test.flags().required = 0;
		return test;
	}
	zp_test& commit(zp_test& test)
	{
		test.flags().commited = 1;
		return test;
	}
	zp_test& one_or_more()
	{
		flags().multi = 1;
		flags().required = 1;
		return *this;
	}
	zp_test& one_or_more(zp_test& test)
	{
		test.flags().multi = 1;
		test.flags().required = 1;
		return test;
	}
	zp_test& optional(zp_test& test)
	{
		test.flags().required = 0;
		return test;
	}
	/*
	zp_test& prop(ctext)
	{
		zp_test_wsp& test = (*z_new zp_test_wsp());
		test.flags().required = 0;
		return test;
	}*/
	zp_test& skipws()
	{
		zp_test_wsp& test = (*z_new zp_test_wsp());
		test.flags().required = 0;
		return test;
	}
	zp_test& ident(ctext prop_name = nullptr);
	virtual void build_test() {}
	virtual zf_prop* find_prop(ctext prop_name);
	//template <class TEST> TEST& test(ctext prop_name = nullptr)
	template <class TEST> zp_test& test(ctext prop_name = nullptr)
	{
		TEST* test = z_new TEST();
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

	zf_prop* prop(ctext prop_name)
	{
		zf_prop* prop = find_prop(prop_name);
		if (!prop)
		{
			Z_ERROR_MSG(zs_feature_not_found, "prop '%s' not found", prop_name);
		}
		return prop;

	}

	//template <class TEST> TEST& test(ctext prop_name = nullptr)
	template <class TARGET> zp_test& obj(ctext prop_name = nullptr);


};

class zp_test_group_anon : public zp_test_group
{
protected:
	Tests _tests;
	Opts  _opts;
public:
	virtual Tests& get_tests() {
		return _tests;
	}
	virtual Opts& get_opts() {
		return _opts;
	}
	virtual zp_test_group & add_test(zp_test *x)
	{
		_tests.push_back(x);
		return *this;
	}

	zp_test_group_anon() { 
		_opts.group_type = group_stage;
		_opts.as_u32=0;
	}
	//zp_test_group_seq(ctext name) { _name = name; }
	virtual ~zp_test_group_anon() {};



};

class zp_test_group_and : public zp_test_group
{
public:
	zp_test_group_and() {}
	virtual ~zp_test_group_and() {};
	virtual z_status test_run(zp_parser* p)
	{

		return zs_ok;

	};

};
class zp_test_group_or : public zp_test_group_anon
{
public:
	zp_test_group_or() { _opts.group_type =  group_or; }
	virtual ~zp_test_group_or() {};
	virtual zp_test_group_or & operator | (zp_test &x)
	{
		add_test(&x);
		return *this;
	};
	virtual zp_test_group_or & operator | (zp_test *x)
	{
		add_test(x);
		return *this;
	};
	virtual zp_test_group_or & operator | (char c)
	{
		zp_test_char* x = z_new zp_test_char(c);
		add_test(x);
		return *this;
	};

};



#define VALUE get_static<zp_test_group_value>()

zp_test_group_or& operator | (zp_test& t1, zp_test& t2);

zp_test_group_or& operator | (zp_test& t1, char x);
zp_test_group_or& operator | (char x, zp_test& t1);




class zp_test_group_seq : public zp_test_group_anon
{
public:
	zp_test_group_seq() {  }
	//zp_test_group_seq(ctext name) { _name = name; }
	virtual ~zp_test_group_seq() {};



};


zp_test_group_seq& operator << (zp_test& t1, zp_test& t2);

zp_test_group_seq& operator <<  (char x, zp_test& t1);
zp_test_group_seq& operator <<  ( zp_test& t1, char x );


typedef z_status(zp_text_parser::*zp_parse_func)();

/*
TODO -  add way of passing in test name so that we can provide error messages like "parse failed, expecting "integer""
*/







#endif