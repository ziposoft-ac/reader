#include "pch.h"

#include "zipolib/z_factory_controller.h"
#include "zipolib/z_filesystem.h"
#include "zipolib/parse_objs.h"



/*________________________________________________________________________

zf_node
________________________________________________________________________*/

z_status zf_node::get_feature_strlist(z_strlist& list,U32 type) 
{
	if (!_factory)
		return Z_ERROR_MSG(zs_internal_error, "factory of node not set");
	return _factory->get_feature_strlist(list, _obj,type);
}

zf_feature* zf_node::get_feature(ctext key)
{
	if (!_factory) return 0;
	return _factory->get_feature(key);



}


bool zf_node::get_child_node(ctext key, zf_node& node)
{
	if (!_factory)
		return false;
	if (_factory->get_child_node(key, *this, node))
		return false;
	return true;

}
//z_factory* zf_node::get_fact()  { return _fact; }
void zf_node::init(z_void_obj* o, z_factory* ft)
{
	_obj = o;
	_factory = ft;
	_name.clear();
}
ctext  zf_node::get_name()
{
	if (_name)
		return _name;
	return _factory->get_name();
}

/*
z_status zf_node::execute_act_ptr(z_ptr_member_func act_addr)
{
	return _factory->execute_act_ptr(_obj, act_addr);



}
*/
z_status zf_node::execute(zf_command_context &cc)
{


	return Z_ERROR_NOT_IMPLEMENTED;




}

void zf_node::dump(zf_feature_type type, zf_operation_flags flags, z_stream& stream, int tab)
{
	get_factory()->dump_features(type, flags, stream, get_obj(), tab);
}
z_status zf_node::assignment(zf_command_line_parser &p)
{
	z_status status = zs_ok;
	p.skip_ws();
	if (p.test_char('<')==zs_ok)
	{
		z_string class_name;
		if (p.test_any_identifier())
			return Z_ERROR_MSG(zs_parse_error, "Expected type name");
		p.get_match(class_name);
		// TODO
		Z_ERROR_MSG(zs_not_implemented, "poly objects not implemented");
		if (p.test_char('>'))
			return Z_ERROR_MSG(zs_parse_error, "Expected '>'");

	}
	if (p.test_char('{'))
		return Z_ERROR_MSG(zs_parse_error, "Expected '{'");
	status = _factory->assignment(p, _obj);
	if (status)
		return status;
	p.skip_ws();
	if (p.test_char('}'))
		return Z_ERROR_MSG(zs_parse_error, "Expected '}'");
	return status;


}
/*
z_status zf_node::assignment(zf_command_context &cc)
{

	return _factory->assignment(cc, _obj);




}*/
z_status zf_node::assignment_add(z_string name, zf_command_line_parser&cc)
{

	return _factory->assignment_add(name,_obj,cc);





}

z_status zf_node::save_cfg(ctext file_name)
{

	z_file_out f;
	z_status status = f.open(file_name);
	if (status)
		return Z_ERROR(status);

	f << '<' << _factory->get_name() << ">{";
	_factory->dump_features(zf_ft_all, ZFF_SAVE, f, _obj, 0);
	f << '}';
	return zs_ok;

}
z_status zf_node::load_cfg(ctext file_name)
{
	return get_factory()->file_load(file_name, get_obj());


}



