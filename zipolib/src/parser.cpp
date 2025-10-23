#include "pch.h"
#include "zipolib/parser.h"
#include "zipolib/z_variable.h"
#include "zipolib/z_filesystem.h"
/*_______________________________________________________________________________________________
zp_parser
*/

#if DEBUG_PARSER
#define PTRC(...) trace(__VA_ARGS__)
#else
#define PTRC(...) 
#endif
z_status zp_parser::load_file(ctext filename)
{
	if (_buff)
		delete _buff;
	size_t len;
	z_status status = z_file_open_and_read(filename, &len, &_buff);
	if (status)
		return status;

	set_source(_buff, len);
	return zs_ok;
}

void zp_parser::trace(ctext pFormat, ...)
{
#if DEBUG_PARSER
	int i = _level;
	while (i--)
		OutputDebugStringA("\t");

	static char buff[128];
	va_list ArgList;
	va_start(ArgList, pFormat);



	int len=vsnprintf(buff, 127, pFormat, ArgList); // Extra space for '\0'
	OutputDebugStringA(buff);
	va_end(ArgList);
	OutputDebugStringA("\n");
#endif
}


void zp_parser::zp_parser_context::cleanup_obj()
{
	if (_created_object)
		if (_obj)
		{
			PT("delete %s=%x", get_fact()->get_name(), _obj);
			get_fact()->delete_void_obj(_obj);
		}

	_created_object = false;
	_obj = 0;

}
z_factory* zp_parser::zp_parser_context::get_fact()
{
	if (_test_obj)
		return _test_obj->get_factory();
	return 0;


}
z_void_obj* zp_parser::zp_parser_context::get_obj(bool create)
{
		if(create && _test_obj)
			if (!_obj)
			{
				
				_obj = _test_obj->new_object();
				PT("create %s=%x", get_fact()->get_name(), _obj);
				_created_object = true;
			}
	return _obj;

}

z_status zp_parser::assign_child_object(zf_prop* prop)
{
	if (!_context_current) 
		return Z_ERROR(zs_internal_error);
	z_void_obj* obj_parent = get_parent_obj(true);
	z_void_obj* obj_child = _context_current->get_obj(true);

	if (_context_current->created_child())
	{
		prop->obj_ptr_set(obj_parent, obj_child);
		PTRC("assigning %s(%x) to prop \"%s\" or %x", 
			_context_current->get_test()->get_name(), obj_child, prop->get_name(), obj_parent);
	}
		
	return zs_ok;
}


z_void_obj* zp_parser::get_current_obj(bool create)
{
	if(!_context_current) return 0;
	return _context_current->get_obj(create);


}
z_void_obj* zp_parser::get_parent_obj(bool create)
{
	if (!_context_current) return 0;

	zp_parser_context* pc = _context_current->get_parent();
	if (!pc)
	{
		Z_ERROR_MSG(zs_internal_error, "should never happen?");
		return 0;

	}
	z_void_obj* obj=pc->get_obj(create);



	return obj;

}
void zp_parser::unwind_obj()
{
	if (_context_current) 

	_context_current->cleanup_obj();




}
z_status zp_parser::start(zp_test* test)
{
	_context_current = &_context_root;
	_context_current->set(test, 0);
	z_status s = test->test_run(this);
	return s;
}


z_status zp_parser::start(zp_test* test, z_void_obj*& obj)
{
	_context_current = &_context_root;
	_context_current->set(test, obj);
	z_status s = test->test_root(this);
	if (s == zs_ok)
		obj = _context_root.get_obj(false);
	return s;
}

z_factory* zp_parser::get_current_fact()
{
	return _context_current->get_fact();
}
