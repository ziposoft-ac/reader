#include "pch.h"

#include "zipolib/z_factory_map.h"

#include "zipolib/z_factory_controller.h"

#include "zipolib/parse_objs.h"






/*________________________________________________________________________

zf_child_obj_map
________________________________________________________________________*/
zf_child_obj_map::zf_child_obj_map(ctext id, ctext name, z_memptr offset, zf_operation_flags flags, ctext desc) :
	zf_prop(&_dummy_var_funcs, id, name, offset, flags, desc), z_factory(name)
{
	_init = true;
	//zf_map_action* act_new =z_new  zf_map_action("new", "new", ZFF_ACT_DEF, "add new object");
	typedef zf_child_obj_map BASECLASS;
	/*
	{
		auto mfunc = &BASECLASS::data_table_json; 
			zf_param_list *params = z_new zf_param_list(); 
			auto parambind = std::bind(mfunc, std::placeholders::_1, std::placeholders::_2);
			zf_cmd* cmd = fact_cmd_add<BASECLASS>(params, parambind, "data_table_json", ZFF_ACT_DEF, "add new object");

	}
	*/
	Z_FACT_ACT(data_table_json,ZFF_ACT_DEF, "Json Data Table Output");








	//Z_FACT_CMD(data_table_json, ZFF_ACT_DEF, "data_table_json",	ZPRM(int, flags, 0, "start immediately", ZFF_PARAM));
	//add_feature("new", act_new);
}
z_status zf_child_obj_map::data_table_json(z_void_obj* vobj_map, z_stream &output)
{
	if (!vobj_map)
	{
		return Z_ERROR_MSG(zs_invalid_conversion, "no object");

	}

	z_factory* fact = get_factory();
	fact->check_init();

	voidMap& map = getMap(vobj_map);
	z_json_stream stream(output, true);
	stream.obj_start();
	stream.key("cols");

	fact->json_array(stream, 0, ZFF_JSON_ARRAY_COLS);

	stream.key("data");
	stream.obj_array_start();

	//bool comma = false;
	for (auto entry : map)
	{
		/*
		if (comma)
			stream << ",\n";
		else
			comma = true;
			*/
		fact->json_array(stream, entry.second, ZFF_JSON_ARRAY_DATA);
	}

	stream.obj_array_end();

	stream.obj_end();


	return zs_ok;
	
	
	
	

}


z_status zf_child_obj_map::get_feature_strlist(z_strlist& list, void* obj, U32 type)
{
	list.clear();
	voidMap& map = getMap(obj);

	for (auto entry : map)
	{
		
		list << entry.first;
	}
	return zs_ok;
}
z_status zf_child_obj_map::get_child_node(ctext key, zf_node &parent, zf_node &node)
{
	voidMap& map = getMap(parent.get_obj());
	z_void_obj* child=map.getobj(key);
	if (!child)
		return zs_child_not_found;

	z_factory* fact= get_factory_list()->get_factory_from_vobj(child);
	node.init(child, fact);
	node.set_name(key);
	return zs_ok;

}
z_status zf_child_obj_map::get_node(zf_node &parent, zf_node &node)
{
	z_void_obj* obj = parent.get_obj();
	void* mem_ptr = (char*)obj + _offset;

	// Node is the parent object plus the this feature.
	// Ugly and confusing
	node.init(obj, this);
	node.set_name(zf_feature::get_name());
	return zs_ok;
}

z_status zf_child_obj_map::evaluate_cc(zf_command_line_parser& cc)
{

	//z_status status;
	if (cc.test_char('=') == zs_ok)
	{
		return zs_operation_not_supported;
	}

	dump( cc.operation(),gz_stdout, cc.get_exec_node().get_obj(), 0);
	return zs_ok;//???
}


z_status  zf_child_obj_map::obj_ptr_set(z_void_obj* parent, z_void_obj* child)
{
	return Z_ERROR_NOT_IMPLEMENTED;

}

z_void_obj* zf_child_obj_map::obj_ptr_create(void* parent)
{

	z_void_obj* child = get_factory()->create_void_obj();
	if (!child)
	{
		Z_ERROR_MSG(zs_child_not_found, "Factory cannot create object?");
	}
	return child;
}
;

z_status zf_child_obj_map::assignment_add(z_string name, void* obj, zf_command_line_parser &cc)
{

	voidMap& map = getMap(obj);
	z_void_obj* child = 0;
	if (cc.operation()&get_flags())
	{
		child = get_factory()->create_void_obj();
		if (!child)
			return Z_ERROR_MSG(zs_child_not_found, "Factory cannot create object?");
	}
	else
		return Z_WARN(zs_operation_not_supported);



	zf_node node(child, get_factory());
	map[name] = (z_void_obj*)child;
	return node.assignment(cc);
}
void   zf_child_obj_map::dump( zf_operation oper, z_stream &stream, void* vobj, int tab)
{
	if (!(oper&_flags)) return;
	if (!vobj)
	{
		Z_ERROR_MSG(zs_internal_error, "vobj == NULL??");
		return;
	}
	//ZT("%s", zf_prop::get_name());
	voidMap& map = getMap(vobj);

	if (oper == ZFF_LIST)
	{
		stream % tab << zf_feature::get_name() << '[' << map.size() << "]\t- " << _desc << "\n";
		return;
	}


	stream% tab << zf_feature::get_name() << "=\n";

	stream% tab++ << "[\n";
	dump_features(zf_ft_obj_list, oper, stream, vobj, tab);
	stream% --tab << "]\n";
}

void   zf_child_obj_map::dump_features(zf_feature_type type, zf_operation oper,z_stream &stream, void* vobj, int tab)
{
	if (!(oper&_flags)) return;
	if (!(type&zf_ft_obj_list)) return;
	if (!vobj)
	{
		Z_ERROR_MSG(zs_internal_error, "vobj == NULL??");
		return;
	}
	ZT("%s", zf_prop::get_name());
	voidMap& map = getMap(vobj);
	

	for (auto entry : map)
	{
		z_void_obj* vobj = (z_void_obj*)entry.second;
		z_factory* fact = get_factory_list()->get_factory_from_vobj(vobj);
		if (oper == ZFF_LIST)
		{
			if (fact)
			{
				fact->check_init();
				stream << '<' << fact->get_name() << ">\t";
			}
			else
			{
				//do not have z_fact for object (you forgot the macro)
				stream << "<unknown>\t";
			}
			stream <<   entry.first << '\n';

		}
		else
		{
			stream%tab << entry.first << "=\n";
			stream% tab++;
			if (fact)
			{
				fact->check_init();
				stream << '<' << fact->get_name() << '>';
				stream << "{\n";
				fact->dump_features(zf_ft_all, oper, stream, vobj, tab);
				stream% --tab << "}\n";
			}
			else
			{
				//do not have z_fact for object (you forgot the macro)
				stream << "<unknown>{}\n";
			}
		}
	}
}

// For the factory
void zf_child_obj_map::json_data_fact(z_json_stream& stream, void* obj, int flags)
{
//	stream.obj_start();
	json_data_children(stream, obj, flags);
//	stream.obj_end();
}
// for the feature
void zf_child_obj_map::json_data(z_json_stream& stream, void* vobj,  int flags)
{

	z_factory* fact = get_factory();

	if (flags&ZFF_JSON_STRUCT)
	{
		stream.keyval("type", "objmap");
		stream.keyval("name", zf_feature::get_name());
		stream.keyval("class",
			(fact ? fact->get_name() : "unknown"));
		stream.key("val");
	}

	json_data_children(stream, vobj, flags);

}


void get_json_from_vmap(z_json_stream& stream, z_factory* default_fact,voidMap& vmap, int flags)
{

	stream.obj_start();
	size_t i = 0;
	bool comma = false;
	for (auto entry : vmap)
	{

		stream.key(entry.first);
		z_void_obj* vobj = (z_void_obj*)entry.second;
		z_factory* fact = get_factory_list()->get_factory_from_vobj(vobj);
		if (fact)
		{
			fact->check_init();
		}
		else
			fact = default_fact;

		if (flags&ZFF_JSON_STRUCT)
		{
			stream.obj_start();
			stream.keyval("type", "obj");
			stream.keyval("name", entry.first);
			stream.keyval("class", fact->get_name());
			stream.key("val");
		}
		if (flags&ZFF_JSON_RECURSE)
			fact->json_data_fact(stream, vobj, flags);
		else
			stream << "{}";
		if (flags&ZFF_JSON_STRUCT)
		{
			stream.obj_end();
		}
	}
	stream.obj_end();

}


// for the feature
void zf_child_obj_map::json_data_children(z_json_stream& stream, void* vobj, int flags)
{

	z_factory* default_fact = get_factory();
	
	// Why did I have this???
	//z_factory* default_fact = zf_child_obj_map::get_factory();


	if (!vobj)
	{
		Z_ERROR_MSG(zs_internal_error, "vobj == NULL??");
		stream << "{}";
		return;
	}



	voidMap& map = getMap(vobj);
	get_json_from_vmap(stream, default_fact,map, flags);

}