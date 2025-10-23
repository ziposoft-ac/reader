#include "pch.h"
#include "zipolib/parser.h"
#include "zipolib/parse_cfg.h"
#include "zipolib/z_factory_map.h"
#include "zipolib/z_factory_vector.h"


ZCLS_VIRT(zp_cfg_base) {}


/*________________________________________________

zp_cfg_val_str_list
________________________________________________*/

ZMETA(zp_cfg_val_str_list)
{
};

TEST(zp_cfg_val_str_list)
{
	_opts.ignore_ws = 1;
	*this << chr('[') << one_or_more(  test<zp_test_quoted_str>()  | ',' ).iws().debug() << ']';
}

/*________________________________________________

zp_cfg_val
________________________________________________*/

ZMETA(zp_cfg_val)
{
};

ZMETA(zp_cfg_val_str)
{
	ZPROP(_val);
};
TEST(zp_cfg_val_str)
{
	get_opts().group_type = group_or;
	*this << test<zp_test_float>("val") <<
		test<zp_test_int>("val") <<
		test<zp_test_quoted_str>("val") <<
		test<zp_test_single_quoted_str>("val") <<
		test<zp_test_cfg_val_str_list>("val") <<
		(str("true") | str("false"))(prop("val"));

	   
	// support this at all?
	//<<	test<zp_test_ident>("val"); //must come last
}


z_status zf_prop::cfg_assign(zp_cfg_val* val, z_factory* fact, z_void_obj* obj_parent)
{
    if(!test_flag(ZFF_LOAD))
    {
        return Z_ERROR(zs_operation_not_supported);
    }
	zp_cfg_val_str* val_str = val->get_string();
	if (!val_str)
	{
		//TODO error?
		return zs_operation_not_supported;
	}
	
	return set_from_string(val_str->_val, obj_parent);

}


/*________________________________________________

zp_cfg_assignment
________________________________________________*/


z_status zp_cfg_assignment::assign(z_factory* fact, z_void_obj* obj)
{

	zf_prop* p = fact->get_feature_t<zf_prop>(_name);
	if (!p)
		return Z_ERROR_MSG(zs_feature_not_found, "\"%s\" has no property %s",
			fact->get_name(),
			_name.c_str());

	//return _val->assign_prop(p, obj);
	return p->cfg_assign(_val, fact, obj);

}


ZMETA(zp_cfg_assignment)
{
	ZPROP(_name);
	ZPROP(_val);
};
TEST(zp_cfg_assignment)
{
	*this << ident("name") << skipws() << commit(chr('=')) << skipws() <<	(
		obj<zp_cfg_val_str>("val") |
		obj<zp_cfg_obj>("val") |
		obj<zp_cfg_empty_array>("val")|
		obj<zp_cfg_obj_vect>("val") |
		obj<zp_cfg_obj_map>("val") 
		
			);
}




ZMETA(zp_cfg_func_call)
{
	ZVECT(_ass);
};
/*________________________________________________

zp_cfg_obj_map
________________________________________________*/

ZMETA(zp_cfg_obj_map_entry)
{
	ZPROP(_name);
	ZOBJ(_obj);
};

TEST(zp_cfg_obj_map_entry)
{
	_opts.ignore_ws = 1;
	*this << ident("name") <<  chr('=')  << obj<zp_cfg_obj>("obj");
}
ZMETA(zp_cfg_obj_map)
{
	ZVECT(_map);
};

ZMETA(zp_cfg_empty_array)
{
};
TEST(zp_cfg_empty_array)
{
	_opts.ignore_ws = 1;
	*this << chr('[') << ']';
}


ZMETA(zp_cfg_obj_vect)
{
	ZVECT(_vect);
};
TEST(zp_cfg_obj_vect)
{
	_opts.ignore_ws = 1;
	*this << str("[") << one_or_more(
		 test<zp_test_int>() << chr('=') <<
		obj<zp_cfg_obj>("vect"))
		<< ']';
}

z_status zf_child_obj_vector::cfg_assign(zp_cfg_val* val, z_factory* fact, z_void_obj* obj_parent)
{

	z_factory* vect_fact = get_factory();
	zf_child_obj_vector::voidVector& vv = getVect(obj_parent);
	z_status status = zs_ok;
	
	// deleting void_objs will cause memory leak
	// so do not call this
	//vv.destroy();
	
	// this converts void_objs up to class to delete them
	// need to support polymorphic objects
	delete_all(vv);
	if (val->get_empty_array())
	{
		return zs_ok;
	}
	zp_cfg_obj_vect* val_vect = val->get_vect();
	if (!val_vect)
	{
		//TODO error?
		return zs_operation_not_supported;
	}
	for (auto cfg_obj : val_vect->_vect)
	{



		z_void_obj* child_obj = 0;
		status = cfg_obj->create_obj(vect_fact, child_obj);
		if (status)
			return status;

		vv.push_back(child_obj);


	}

	return zs_ok;
}

TEST(zp_cfg_obj_map)
{
	_opts.ignore_ws = 1;
	*this << str("[") <<  one_or_more(
		obj<zp_cfg_obj_map_entry>("map"))
		 << ']';
}

z_status zf_child_obj_map::cfg_assign(zp_cfg_val* val, z_factory* fact, z_void_obj* obj_parent)
{
	z_status status = zs_ok;
	z_factory* map_fact = get_factory();
	voidMap& vm = getMap(obj_parent);
	if (val->get_empty_array())
	{
		vm.delete_all();
		return zs_ok;
	}

	zp_cfg_obj_map* val_map = val->get_map();
	if (!val_map)
		return zs_operation_not_supported;


	for (auto config_entry : val_map->_map)
	{

		ctext name = config_entry->_name;
		zp_cfg_obj& cfg_obj = config_entry->_obj;
		z_void_obj* child_obj = vm.getobj(name);


		if (child_obj)
		{
			z_factory* fact = get_factory_list()->get_factory_from_vobj(child_obj);
			status = cfg_obj.assign(fact, child_obj);
			if (status) // TODO- quit??? warning? error?
				return status;
		}
		else
		{
			status = cfg_obj.create_obj(map_fact, child_obj);
			if (status)
				return status;

			vm[name] = child_obj;
		}
	}
	return zs_ok;

}





/*________________________________________________

					zp_cfg_obj
________________________________________________*/

ZMETA(zp_cfg_obj)
{
	ZPROP(_type);
	ZVECT(_ass);
	ZVECT(_funcs);
	//ZOBJ(_bean2);
};

TEST(zp_cfg_obj)
{
	//<< z_new zp_test_obj_contents()
	// << obj<zp_test_obj_contents>("_propname")
	_opts.ignore_ws = 1;
	*this << optional(commit(chr('<'))
		<< ident("type") << '>')
		<< commit(chr('{'))
		<< zero_or_more(obj<zp_cfg_assignment>("ass") | ';' | obj<zp_cfg_func_call>("funcs")  ).iws()
		<< '}';
}
TEST(zp_cfg_func_call)
{
	*this << ident() << commit(chr('('))
		<< zero_or_more(obj<zp_cfg_assignment>("ass") | ',' | test<zp_test_wsp>())
		<< ')';

}



z_status zp_cfg_obj::create_obj(z_factory* fact, z_void_obj*& obj)
{
	if (_type)
	{
		fact = get_factory_by_name(_type);
		if (!fact)
			return Z_ERROR_MSG(zs_item_not_found, "type \"%s\" is unknown", _type.c_str());
	}
	obj=fact->create_void_obj();
	if (!obj)
	{
		// this can happen if it is a pure virtual factory
		return Z_ERROR(zs_cannot_create_virtual_obj);
	}
	return assign(fact, obj);

	

}
z_status zf_child_obj_base::cfg_assign(zp_cfg_val* val, z_factory* fact, z_void_obj* obj_parent)
{
	zp_cfg_obj* val_obj = val->get_obj();
	if (!val_obj)
		return zs_operation_not_supported;

	z_void_obj* obj_child = obj_ptr_get(obj_parent);
	if (!obj_child)
	{
		// this can happen if it is a pure virtual factory
		obj_child = obj_ptr_create(obj_parent);

	}
	if (!obj_child)
		return zs_cannot_create_virtual_obj;

	z_factory* child_fact = get_factory();

	z_status status = val_obj->assign(child_fact, obj_child);
	if (status)
		Z_ERROR(status);
	return status;

}

z_status zp_cfg_obj::assign(z_factory* fact, z_void_obj* obj)
{

	for (auto ass : _ass)
	{
		/*
		zf_prop* p = fact->get_feature_type<zf_prop>(ass->_name);
		if (!p)
			continue; // TODO - unknown feature. Error? warning?
		z_status status=p->set_from_string(ass->_val, obj);

			*/
		z_status status = ass->assign(fact, obj);


	}

	return zs_ok;

}


