#ifndef z_parser_h
#define z_parser_h
#include "zipolib/parse_objs.h"
#if defined(DEBUG) && defined(WINDOWS)
#define DEBUG_PARSER 0
#else
#undef DEBUG_PARSER 
#endif

#if DEBUG_PARSER
#define PT(...) p->trace(__VA_ARGS__)
#else
#define PT(...) 
#endif
class zp_parser : public zp_text_parser
{
	class zp_parser_context
	{



		bool _created_object;
		z_void_obj* _obj;

		zp_test* _test_obj;	
		zp_parser* p;
	public:

		

		zp_parser_context* _parent;
		zp_parser_context* _child;
		zp_parser_context(zp_parser* parser)
		{
			p = parser;
			_child = 0;
			_parent = 0;
			_obj = 0;
			_test_obj = 0;
			_created_object = false;
		}

		virtual ~zp_parser_context()
		{
			if (_child)
				delete _child;
			_child = 0;
		}
		//zf_node _node;
		void set(zp_test* test, z_void_obj* obj)
		{
			_test_obj = test;
			_obj = obj;
			_created_object = false;
		}
		bool created_child() {
			return _created_object;
		}
		zp_test* get_test() {
			return _test_obj ;
		}
		zp_parser_context* get_child(zp_test* test, z_void_obj* obj)
		{
			if (!_child)
			{
				_child = z_new zp_parser_context(p);
				_child->_parent = this;
			}
			_child->set(test, obj);
			return _child;
		}
		zp_parser_context* get_parent()
		{
			return _parent;
		}

		z_factory* get_fact();
		z_void_obj* get_obj(bool create);
		void cleanup_obj();


	};

private:
	int _level{ 0 };
	char* _buff;
	zp_parser_context _context_root;
	zp_parser_context* _context_current;
protected:

public:

	zp_parser() : _context_root(this)
	{
		_buff = 0;
		_context_current = 0;
	}
	virtual ~zp_parser()
	{
		cleanup();
	};
	void level_down() { _level++; }
	void level_up() { _level--;  }
	void trace(ctext pFormat, ...);
	void print(int p)
	{

	}
	z_status assign_child_object(zf_prop* prop);
	z_void_obj* get_current_obj(bool create);
	z_void_obj* get_parent_obj(bool create);
	void unwind_obj();
	z_factory* get_current_fact();
	//z_status run_test(zp_test* test);
	void context_push(zp_test* test, z_void_obj* obj)
	{
		//ug ugly
		if (!_context_current)
		{
			_context_root.set(test, obj);
			_context_current = &_context_root;
		}
		else
		_context_current = _context_current->get_child(test, obj);
	}
	void context_pop()
	{
		if (!_context_current)
		{
			Z_ERROR_MSG(zs_internal_error, "bad call to contect_pop()");
			return;
		}
		if(_context_current!= &_context_root)
			_context_current = _context_current->get_parent();
	}
	z_status start(zp_test* test, z_void_obj*& obj);
	z_status start(zp_test* test);

	z_status load_file(ctext filename);
	void cleanup()
	{
		if (_buff)
			delete _buff;
		_buff = 0;
	}



};
template <z_status(zp_text_parser::*PARSE_FUNC)()> class zp_simple_test_t : public zp_test
{
public:

	zp_simple_test_t()
	{

	}

	virtual z_status test_run(zp_parser* p)
	{
		z_status s = (p->*PARSE_FUNC)();
		if (s == zs_ok)
			return test_pass(p);
		return s;

	}

};

const char txt_quoted_string[] = "quoted string";
typedef zp_simple_test_t<&zp_parser::test_code_string> zp_test_quoted_str;
typedef zp_simple_test_t<&zp_parser::test_floating_point> zp_test_float;
typedef zp_simple_test_t<&zp_parser::test_integer> zp_test_int;
typedef zp_simple_test_t<&zp_parser::test_single_quoted_string> zp_test_single_quoted_str;
typedef zp_simple_test_t<&zp_parser::test_any_identifier> zp_test_ident;

//class zp_test_array_string : public zp_test_array<zp_test_quoted_str>{public:};

#endif