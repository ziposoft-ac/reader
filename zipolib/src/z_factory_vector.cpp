#include "pch.h"

#include "zipolib/z_factory_vector.h"

#include "zipolib/z_factory_controller.h"

#include "zipolib/parse_objs.h"




/*________________________________________________________________________

zf_child_obj_vector
________________________________________________________________________*/

z_status zf_child_obj_vector::get_feature_strlist(z_strlist& list, void* obj, U32 type)
{
	list.clear();
	voidVector& vect = getVect(obj);
	int i = 0;
	for (auto entry : vect)
	{
		
		list << z_string(i++);
	}
	return zs_ok;
}
z_status zf_child_obj_vector::get_child_node(ctext key, zf_node &parent, zf_node &node)
{
	voidVector& vect = getVect(parent.get_obj());
	int i = atoi(key);
	z_void_obj* child = vect[i];
	if (!child)
		return zs_child_not_found;
	node.init(child, get_factory());
	node.set_name(key);
	return zs_ok;

}
z_status zf_child_obj_vector::get_node(zf_node &parent, zf_node &node)
{
	z_void_obj* obj = parent.get_obj();
	void* mem_ptr = (char*)obj + _offset;

	// Node is the parent object plus the this feature.
	// Ugly and confusing
	node.init(obj, this);
	node.set_name(zf_feature::get_name());
	return zs_ok;
}

z_status zf_child_obj_vector::evaluate_cc(zf_command_line_parser& cc)
{

	//z_status status;
	if (cc.test_char('=') == zs_ok)
	{
		return zs_operation_not_supported;
	}

	dump( cc.operation(),gz_stdout, cc.get_exec_node().get_obj(), 0);
	return zs_ok;//???
}
z_status  zf_child_obj_vector::obj_ptr_set(z_void_obj* parent, z_void_obj* child)
{
	getVect(parent).add_void(child);
	return zs_ok;//???;

}

z_void_obj* zf_child_obj_vector::obj_ptr_create(void* parent)
{

	z_void_obj* child = get_factory()->create_void_obj();
	if (!child)
	{
		Z_ERROR_MSG(zs_cannot_create_virtual_obj, "Factory cannot create object");
		return 0;
	}
	return child;
}

void zf_child_obj_vector::delete_all(voidVector& vv) const
{
	for (auto vo : vv)
	{
		delete_void_obj(vo);
	}
	vv.clear();
}



z_status zf_child_obj_vector::assignment_add(z_string name, void* obj, zf_command_line_parser &cc)
{

	voidVector& vect = getVect(obj);
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
	vect[name] = (z_void_obj*)child;
	return node.assignment(cc);
}
void   zf_child_obj_vector::dump( zf_operation oper, z_stream &stream, void* vobj, int tab)
{
	if (!(oper&_flags)) return;
	if (!vobj)
	{
		Z_ERROR_MSG(zs_internal_error, "vobj == NULL??");
		return;
	}
	ZT("%s", zf_prop::get_name());
	voidVector& vect = getVect(vobj);
	if (oper == ZFF_LIST)
	{
		stream << zf_feature::get_name() << '[' << vect.size() << "]\t- " << _desc << "\n";
		return;
	}
	stream% tab << zf_feature::get_name() << "=\n";

	stream% tab++ << "[\n";
	dump_features(zf_ft_obj_list, oper, stream, vobj, tab);
	stream% --tab << "]\n";
}

void   zf_child_obj_vector::dump_features(zf_feature_type type, zf_operation oper,z_stream &stream, void* vobj, int tab)
{
	if (!(oper&_flags)) return;
	if (!(type&zf_ft_obj_list)) return;
	if (!vobj)
	{
		Z_ERROR_MSG(zs_internal_error, "vobj == NULL??");
		return;
	}
	voidVector& vect = getVect(vobj);

	size_t i = 0;
	for (auto entry : vect)
	{
		stream%tab << i++ << "=\n";
		stream% tab++;


		z_factory* fact = get_factory_list()->get_factory_from_vobj(entry);
		if (!fact)
			fact = get_factory();
		fact->check_init();
		stream << '<' << fact->get_name() << '>';

		stream << "{\n";
		fact->dump_features(zf_ft_all, oper, stream, entry, tab);
		stream% --tab << "}\n";
	}

}

void zf_child_obj_vector::json_data(z_json_stream& stream, void* vobj,  int flags)
{


	if (flags&ZFF_JSON_STRUCT)
	{
		stream.keyval("type", "objvect");
		stream.key("val");
	}

	if (!vobj)
	{
		Z_ERROR_MSG(zs_internal_error, "vobj == NULL??");
		stream << "[]";
		return;
	}
	voidVector& vect = getVect(vobj);
	stream.obj_array_start();

	size_t i = 0;
	//bool comma = false;
	for (auto entry : vect)
	{
		/*
		if (comma)
			stream << ",\n";
		else
			comma = true;
			*/
		z_factory* fact = get_factory_list()->get_factory_from_vobj(entry);
		fact->check_init();
		fact->json_data_fact( stream, entry,  flags);
	}
	stream.obj_array_end();
	return ;


}