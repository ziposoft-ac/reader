#include "pch.h"
#include "zipolib/parser.h"


z_status zp_test_obj::test_root(zp_parser* p)
{


	z_status status = zp_test_group::test_run(p);
	if (status) //failed
	{
		PT("%s failed,unwind", get_name());
		p->unwind_obj();
	}
	return status;

}


z_status zp_test_obj::test_run(zp_parser* p)
{
	//PT("%s starting", get_name());
	// this should only be called for a sub object
		//return Z_ERROR_MSG(zs_feature_not_found, "no property");
	zf_prop* prop = get_prop();

	z_void_obj* obj_parent = 0;
	z_void_obj* obj_child = 0;


	obj_parent = p->get_current_obj(false);
	if(obj_parent)
		if(prop)
			obj_child =(z_void_obj*)prop->obj_ptr_get(obj_parent);


	p->context_push(this, obj_child);
	z_status status = zp_test_group::test_run(p);
	if (status) //failed
	{
		//PT("%s failed,unwind", get_name());
		p->unwind_obj();
	}
	else
	{
		if (prop)
		{
			p->assign_child_object(prop);

		}
		else
		{
			PT("ERROR. no parent prop, throwing away object");
			p->unwind_obj();
		}

	}
	p->context_pop();
	return status;


}
