#include "pch.h"

#include "zipolib/z_factory_controller.h"

#include "zipolib/z_filesystem.h"
#include "zipolib/parse_cfg.h"



/*________________________________________________________________________

z_factory_map
________________________________________________________________________*/

z_factory* z_factory_map::get_factory(ctext name)
{
	z_factory* f;
	get(name, f);
	if (f)
		f->check_init();
	return f;

}

void z_factory_map::add_factory(ctext name, z_factory* fact)
{
	if (exists(name))
	{
		// TODO throw??
		Z_ERROR_MSG(zs_already_exists, "class \"%\" already exists.", name);
		return;
	}
	_type_map[fact->get_type_index()] = fact;
	this->add(name, fact);

}

z_factory* z_factory_map::get_factory_by_type(std::type_index ti)
{
	if (_type_map.find(ti) == _type_map.end())
		return 0;

	return _type_map[ti];

}
z_factory* z_factory_map::get_factory_from_vobj(const z_void_obj* vo)
{
	if (!vo)
		return 0;

	// TODO some sort of check that vo has a vtable. otherwise this crashes!
	std::type_index ti = std::type_index(typeid(*vo));

	z_factory* fact= get_factory_by_type(ti);
	if(fact)
		fact->check_init();
	return fact;

}
z_factory* get_factory_from_vobj(const z_void_obj* vo)
{
	return get_factory_list()->get_factory_from_vobj(vo);

}

z_factory_map* get_factory_list()
{
	return get_factory_list_t<z_factory_map_def>();
}
z_factory* get_factory_by_name(ctext name) {
	return get_factory_list()->get_factory(name);
}

/*________________________________________________________________________

z_factory
________________________________________________________________________*/

z_factory::z_factory(ctext name)
{
	// This will be run at global init whether factory is used or not
	// so keep this lean
	_init = false;
	_name = name;
	//_base_factory = factory_base;
	//get_factory_list()[name] = this;
}
z_status z_factory::execute_act_ptr(void * vobj, z_ptr_member_func act_addr)
{
	// zf_child_obj_map does not implement this
	return Z_ERROR_NOT_IMPLEMENTED;
}

z_status z_factory::execute_act_stream_ptr(void * vobj, z_ptr_member_func_stream act_addr, z_stream & stream)
{
	// zf_child_obj_map does not implement this
	return Z_ERROR_NOT_IMPLEMENTED;
}

z_status z_factory::execute_act_json_ptr(void * vobj, z_ptr_member_func_json act_addr, z_json_stream & stream, z_json_obj & params)
{
	// zf_child_obj_map does not implement this
	return Z_ERROR_NOT_IMPLEMENTED;
}

#if 0
zf_action* z_factory::add_act_params(ctext id, ctext name, z_ptr_member_func act_addr, zf_operation_flags flags, ctext desc, int num_params, ...)
{

	// TODO!! This is not actually working at the moment
	int i;
	zf_action* action = add_mfunc(id, name, act_addr, flags, desc);
	va_list ArgList;
	va_start(ArgList, num_params);
	for (i = 0; i < num_params; i++)
	{
		zf_prop* p = va_arg(ArgList, zf_prop*);
		if (!p)
		{
			Z_ERROR_MSG(zs_bad_parameter, "Cannot add parameter #%d to action '%s'\n", i, id);
			return 0;
		}
		// Z_ASSERT(0);
		 //action->_params[p->get_name()] = p;
	//	action->_params.add(p);
	}
	return action;
}
#endif
z_status z_factory::get_feature_strlist(z_strlist& list, void* obj, U32 type)
{
	check_init();

	for (auto i : _features)
	{
		if ((i.second->get_type()&type))
			i.second->add_to_list(list, obj);


	}
	return zs_ok;
}
z_status z_factory::get_child_node(ctext key, zf_node &parent, zf_node &node)
{

	zf_feature* f = _features.getobj(key);
	if (!f)
		return zs_child_not_found;
	return f->get_node(parent, node);

}

zf_feature* z_factory::get_feature_idx(size_t idx)
{

	Z_ASSERT(_init);
	zf_feature* feat = 0;


	return _features.get_at(idx);


}
zf_feature* z_factory::get_feature(ctext key)
{
	Z_ASSERT(_init);
	zf_feature* feat = 0;
	return _features.getobj(key);
}

z_status z_factory::assignment( zf_command_line_parser& p,  void* obj)
{
	z_string feature_str;
	z_status status = zs_ok;
	zf_node node((z_void_obj*)obj, this);
	while (status == zs_ok)
	{
		p.skip_ws();
		status = p.test_any_identifier();
		if (status == zs_ok)
		{
			p.get_match(feature_str);
		}
		else
			break;
		//return Z_ERROR_MSG(zs_parse_error, "Expected Identifier");

		zf_action* action = get_feature_t<zf_action>(feature_str);
		if (action)
		{
			status = action->load_params(p);
		}
		else
		{
			zf_feature* f = get_feature(feature_str);
			if (p.test_char('=') != zs_matched)
			{
				return Z_ERROR_MSG(zs_parse_error, "Expected '='");

			}

			if (status == zs_ok)
			{
				zf_node child;
				if (get_child_node(feature_str, node, child) == zs_ok)
				{
					status = child.assignment( p);
					/*
					if (f->get_flags() & oper)
						status = child.assignment(oper, cc);
					else
						status = zs_ok; //skipping.
						*/
				}
				else
				{

					if (!f)
					{
						status = node.assignment_add( feature_str, p);
						if (status == zs_operation_not_supported)
							return Z_ERROR_MSG(zs_parse_error, "Unknown feature: %s", feature_str.c_str());
					}
					else
					{
						
						status = f->assignment( p, node);

						
					}
				}
			}
		}
	}
	if (status == zs_eof)
		return zs_ok;
	if (status != zs_no_match)
	{
		return Z_ERROR(status);

	}
	return zs_ok;

}
void z_factory::json_structure(z_json_stream& stream, void* obj,  int flags)
{
	stream.obj_start();
	for (auto i : _features)
	{
		if (!(i.second->get_flags()&ZFF_JSON_STRUCT))
			continue;

		stream.key(i.first);
		stream.obj_start();
		i.second->json_structure(stream, obj,  flags);
		stream.obj_end();

	}
	stream.obj_end();
}

void z_factory::dump_features(zf_feature_type type, zf_operation oper, z_stream & stream, void * obj, int tab)
{
	for (auto i : _features)
	{
		if (type & i.second->get_type())
			i.second->dump(oper, stream, obj, tab);
	}
}

void z_factory::save(z_stream & stream, void * obj, int tab)
{
	for (auto i : _features)
	{
		i.second->save(stream, obj, tab);
	}
}

void z_factory::json_array(z_json_stream& stream, void* obj,  int flags)
{
	
	stream.array_start();
	bool comma = false;
	for (auto i : _features)
	{
		if (!(i.second->get_flags()&ZFF_JSON_DATA)) // compare ZFF_JSON_DATA
			continue;
		if (comma)
			stream << ',';

		if (obj)
			i.second->json_data(stream, obj, flags);
		else
			//output the column (name of prop)
			stream << '\"' << i.first << "\"";

		comma = true;

	}
	stream.array_end();
}
void z_factory::json_data_fact(z_json_stream& stream, void* obj, int flags)
{


	stream.obj_start();
	for (auto i : _features)
	{
		if (!(i.second->get_flags()&flags))
			continue;
		stream.key(i.first);

		if (flags&ZFF_JSON_STRUCT)
			stream.obj_start();

		i.second->json_data(stream, obj, flags);

		if (flags&ZFF_JSON_STRUCT)
			stream.obj_end();
	}
	stream.obj_end();
}


z_status z_factory::file_load(ctext file_name, z_void_obj * obj)
{
	z_status status = zs_not_implemented;

	ZT("file=%s", file_name);
	status = z_file_exists(file_name);
	if (status)
	{
		return Z_ERROR_MSG(status, "Cannot find file %s", file_name);

	}
	size_t buff_length;
	char* buffer;
	status = z_file_open_and_read(file_name, &buff_length, &buffer);
	if (status)
		return Z_ERROR_MSG(status, "Could not open file \"%s\"", file_name);


	zp_test_obj_t<zp_cfg_obj> test;

	zp_cfg_obj *p_config_obj = test.parse(buffer);
	z_delete []buffer;
	if (!p_config_obj)
	{
		return zs_parse_error;

	}
	status=p_config_obj->assign(this, obj);
	delete p_config_obj;

	return status;



	}

zf_feature * z_factory::add_feature(ctext key, zf_feature * feature)
{
	z_string name = key;
	name.trim_leading_underscore();
	//ctext name = (alias ? alias : key);
	if (_features.exists(name))
	{
		Z_ERROR_MSG(zs_already_exists, "Feature \"%s\" already exists!", name.c_str());
	}
	_features[name] = feature;
	return feature;
}
