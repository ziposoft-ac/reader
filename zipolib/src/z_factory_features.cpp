#include "pch.h"

#include "zipolib/z_factory_controller.h"

ctext ZFF_PARSE_STRING = "PARSE";

ZCLS_VIRT(zf_feature)
{
	ZPROP(_desc);
};



/*________________________________________________________________________

zf_feature
________________________________________________________________________*/

zf_feature::zf_feature()
{

}
zf_feature::zf_feature(ctext id, ctext name, zf_operation_flags flags, ctext desc)
{
	_flags = flags;
	_name = name;
	_name.trim_leading_underscore();
	_desc = desc;

}

z_status zf_feature::evaluate_cc( zf_command_line_parser &cc)
{
	return zs_skipped;
};
z_status zf_feature::evaluate(void* vobj)
{
	return zs_skipped;

}
zf_user_level zf_feature::get_user_level() {
	return (zf_user_level)((_flags & ZFF_USER_LEVEL_MASK)>>24);
}
void  zf_feature::set_user_level(zf_user_level lvl) {
	_flags &= (~ZFF_USER_LEVEL_MASK);
	_flags &= (lvl << 24);

}

/*________________________________________________________________________

zf_action
________________________________________________________________________*/
zf_mfunc::zf_mfunc(ctext id, ctext name, z_ptr_member_func offset, zf_operation_flags flags, ctext desc)
	: zf_action(id, name, flags, desc)
{
	//ZT("act id=%s name=%s offset=%llx flags=%llx\n",id,name,offset,flags);
	_offset = offset;
}
zf_mfunc_stream::zf_mfunc_stream(ctext id, ctext name, z_ptr_member_func_stream offset, zf_operation_flags flags, ctext desc)
	: zf_action(id, name, flags, desc)
{
	//ZT("act id=%s name=%s offset=%llx flags=%llx\n",id,name,offset,flags);
	_offset = offset;
}
zf_mfunc_json::zf_mfunc_json(ctext id, ctext name, z_ptr_member_func_json offset, zf_operation_flags flags, ctext desc)
	: zf_action(id, name, flags, desc)
{
	//ZT("act id=%s name=%s offset=%llx flags=%llx\n",id,name,offset,flags);
	_offset = offset;
}


z_status zf_mfunc::execute(zf_command_context &cc)
{
	return cc.get_seclected_factory()->execute_act_ptr(cc.get_selected_obj() , _offset);
}
z_status zf_mfunc_stream::execute(zf_command_context &cc)
{
//	FUCK TODO - pass stream into action
		return cc.get_seclected_factory()->execute_act_stream_ptr(cc.get_selected_obj(), _offset,cc.get_output_stream());
}
z_status zf_mfunc_json::load_params(zf_command_line_parser & cc)
{
	z_status status;

	status = cc.test_char('{');
	if (status == zs_eof)
		return zs_ok;// no params
	if (status == zs_no_match)
	{
		Z_THROW( "\"%s\" expecting json object {} ", get_name());
	}
	cc.get_json_params().clear();

	status= cc.parse_json_obj_contents(cc.get_json_params());
	if (status)
		return status;
	cc.skip_ws();
	status = cc.test_char('}');
	if (status)
		Z_THROW("Expected '}'");
	return status;
}

z_status zf_mfunc_json::execute(z_json_stream& stream, z_factory* fact, z_void_obj* obj, z_json_obj& params)
{

	return fact->execute_act_json_ptr(obj, _offset, stream, params);


}

z_status zf_mfunc_json::execute(zf_command_context &cc)
{
	//	FUCK TODO - pass stream into action
	z_json_stream* js = cc.get_json_stream();
	return cc.get_seclected_factory()->execute_act_json_ptr(cc.get_selected_obj(), _offset, *js,cc.get_json_params());
}

void zf_action::json_data(z_json_stream& stream, void* obj,  int flags)
{
	if (flags&ZFF_JSON_STRUCT)
	{
		stream.keyval("type", "act");
		stream.keyval("name", get_name());
		stream.keyval("desc", get_desc());

	}

}

void  zf_action::dump(zf_operation oper, z_stream &stream, void* obj, int tab)
{
	if (!(_flags  &oper))
		return;
	z_string str;
	stream.indent(tab);
	stream << get_name() << '(';// (oper == ZFF_SAVE ? "(" : "(");
	//size_t param_index = 0;
    z_string val;
    bool comma=false;
    if(_params)
        for(auto param : *_params)
        {
            if (comma)
                stream << ',';
            else
                comma=true;
            z_status status = param->get_as_string(val, 0);
            stream << param->get_name() << '=' << val;
        }

	stream << ")\n";
};

z_status zf_action::load_params(zf_command_line_parser& cc)
{


	zp_text_parser& parser = cc;
	z_status status;
	z_string error_msg;
	size_t param_count = (_params ? _params->size() : 0);

	parser.skip_ws();
	bool parens = true;
	status = parser.test_char('(');

	if (status == zs_eof)
		return zs_ok;// no params
	if (status == zs_no_match)
	{
		parens = false;

	}
	if (param_count == 0)
	{
		parser.print_context(cc.get_output_stream());
		Z_THROW_MSG(zs_bad_parameter, "\"%s\" takes no parameters (found char 0x%x)", get_name(), parser.get_char_under_test());
	}
	z_string ident;
	// First see if there are named params

	bool try_by_position = true;
	ctext index=parser.get_index();
	while (1)
	{
		status = parser.test_any_identifier();	
		if (status != zs_ok)
			break;
		parser.get_match(ident);

		zf_param* p = _params->get_param_by_name(ident);
		if (!p)
		{
			
			if (try_by_position)
			{
				//Fall down and reparse the first param.
				parser.set_index(index);
				break;
			}
			Z_THROW_MSG(zs_bad_parameter, "Unknown parameter: %s", ident.c_str());
		}
		try_by_position = false;
		parser.skip_ws();
		if (parser.test_char('='))
		{
			Z_THROW_MSG(zs_parse_error, "Expecting '=' assignment");
		}

		status = p->assignment(cc, cc.get_exec_node());
		if (status)
		{
			Z_THROW("Bad assignment for parameter \"%s\"",p->get_name());
		}

		if (parser.test_char(',') == zs_ok)
		{
			continue;
		}
		break;

	}


	if (try_by_position)
	{

		// load params by position
		for(auto param : *_params)
		{
			z_string s;
			status = param->assignment( cc, cc.get_exec_node());
			if (status)
			{
				Z_THROW("Bad input for parameter \"%s\"",
					param->get_name());


				return status;
			}
				
			status = parser.test_char(',');
			if (status)
			{
				status = zs_ok;//less params is ok
				break;
			}

		}
	}
	if (parens)
	{
		parser.skip_ws();
		status = parser.test_char(')');
		if (status)
			Z_THROW_MSG(zs_parse_error, "Expected ')'");
	}
	return zs_ok;
}

void zf_param_list::add(zf_param* param)
{
#ifdef ARM

    this->push_back(param);
#else
    this->push_front(param);
#endif
}


z_status zf_action::evaluate_cc(zf_command_line_parser& cc)
{
	z_string s;
	z_string dbg;
	z_status status = load_params(cc);
	if (status)
		return status;

#if 0
	dbg << get_name() << '(';
	size_t i;

	for (i = 0; i < _params.size(); i++)
	{

		if (i)
			dbg << ',';
		dbg << _params[i]->get_name();
		dbg << "=";
		_params[i]->get_string_val(s, obj._obj);
		dbg << s;

	}

	dbg << ")\n";
	ZT("%s\n", dbg.c_str());
#endif
	if (cc.operation() & ZFF_EXE)
	{
		status = execute(cc);
		return status;
	}
		// We are loading only, don't execute
	return zs_ok;
	
}

/*________________________________________________________________________

zf_child_obj
________________________________________________________________________*/

z_status zf_child_obj_base::get_feature_strlist(z_strlist& list, void* obj)
{
	return get_factory()->get_feature_strlist(list, obj);
}


z_status zf_child_obj_base::evaluate_cc(zf_command_line_parser& parser)
{

	z_void_obj* parent = parser.get_selected_obj();
	z_void_obj* mem_ptr = obj_ptr_get(parent);
	if (!mem_ptr)
	{
		return Z_ERROR(zs_child_not_found);
	}
	if (parser.test_char('=') == zs_ok)
	{
		if (!(parser.operation()&_flags))
			return zs_access_denied;
		//status = _var_funcs->load_from_parser(parser, mem_ptr);
		return zs_not_implemented;
	}
	
	//dump(ZFF_SHOW, gz_stdout, obj, 0);
	return zs_ok;//???
}
z_factory* zf_child_obj_base::get_factory_from_vobj(z_void_obj* vo)
{
	return get_factory();


}
void   zf_child_obj_base::dump(zf_operation oper, z_stream &stream, void* parent, int tab)
{
	z_void_obj* mem_ptr = obj_ptr_get(parent);
	if (!mem_ptr)
	{
		// Child ptr could be zero
		//Z_ERROR(zs_child_not_found);
		return;
	}
	z_factory* fact = get_factory_from_vobj(mem_ptr);
	if (!fact)
	{
		Z_ERROR(zs_wrong_object_type);
		return;
	}
	stream.indent(tab);
	if (oper == ZFF_LIST)
	{
		stream << get_name() << "{}\t- "<< _desc <<"\n";
		return;
	}
	

	stream << get_name() << "=\n";
	stream% tab++ << "{\n";

	fact->dump_features(zf_ft_all, oper, stream, mem_ptr, tab);
	stream% --tab << "}\n";
}
// TODO combine these 2 dumps


z_factory* zf_child_obj_ptr::get_factory_from_vobj(z_void_obj* vo)
{
	z_factory* fact = get_factory_list()->get_factory_from_vobj(vo);
	return fact;


}


z_status zf_child_obj_base::get_node(zf_node &parent, zf_node &node)
{
	void* parent_obj = parent.get_obj();
	z_void_obj* mem_ptr = (z_void_obj*) obj_ptr_get(parent_obj);
	if (!mem_ptr)
	{
		//TODO!
		// if this prop is an obj pointer, NULL is a valid value
		//return Z_ERROR(zs_child_not_found);
	}
	z_factory* fact = get_factory();
	if (!fact)
	{
		return Z_ERROR(zs_wrong_object_type);
	}
	node.init(mem_ptr, fact);
	node.set_name(get_name());

	return zs_ok;
}

void zf_child_obj_base::json_data(z_json_stream& stream, void* obj,  int flags)
{
	z_factory* fact = 0;
	z_void_obj* mem_ptr = obj_ptr_get(obj);
	if (mem_ptr)
	{
		fact = get_factory_from_vobj(mem_ptr);
		if (!fact)
			fact = get_factory();
		if (!fact)
			Z_ERROR(zs_wrong_object_type);

	}


	if (flags&ZFF_JSON_STRUCT)
	{
		stream.keyval("name", zf_feature::get_name());

		stream.keyval("type", "obj");
		stream.keyval("class",
			(fact ? fact->get_name() : "unknown"));

		stream.key("val");
	}
	if(flags&ZFF_JSON_RECURSE)
		if (fact)
		{
			fact->json_data_fact(stream, mem_ptr, flags);
			return;
		}

	stream << "{}";





}

/*________________________________________________________________________

zf_stat
________________________________________________________________________*/

z_status zf_stat::evaluate_cc(zf_command_line_parser&cc)
{
	void* obj = cc.get_exec_node().get_obj();

	if (!(ZFF_SHOW&_flags))
		return zs_access_denied;
	z_string str;
	get_as_string(str, obj);
	cc.get_output_stream() << get_name() << "=" << str << "\n";
	return zs_ok;//???
}
void   zf_stat::dump(zf_operation flags, z_stream &stream, void* obj, int tab)
{
	if (!(_flags  &flags))
		return;
	z_string str;
	stream.indent(tab);
	get_as_string(str, obj);
	stream << get_name() << "=" << str << "\n";

}

void zf_stat::json_data(z_json_stream& stream, void* obj,  int flags)
{
	if (flags&ZFF_JSON_STRUCT)
	{
		stream.keyval("type", "stat");
		stream.key("val");
	}

	z_string s;
	get_as_string(s,obj);
	stream.val(s);
}

/*________________________________________________________________________

zf_var
________________________________________________________________________*/

zf_var::zf_var(z_variable_base* var, ctext id, ctext name, zf_operation_flags flags, ctext desc)
	: zf_feature(id, name, flags, desc)
{
	_var_funcs = var;
}

z_status zf_var::assignment(zf_command_line_parser &cc, zf_node &node)
{
    if(!(_flags&ZFF_WRITE))
    {
        return Z_ERROR(zs_operation_not_supported);
    }


	if (!(cc.operation()&_flags))
	{
	    // TODO - What the fuck?
		_var_funcs->load_from_parser(cc, 0);
		Z_THROW_MSG(zs_access_denied, "Property \"%s\" cannot be assigned", get_name());
	}
	void* obj = node.get_obj();
	void* mem_ptr = get_var_ptr(obj);
	z_status status = _var_funcs->load_from_parser(cc, mem_ptr);

	return status;
}
z_status zf_var::evaluate_cc(zf_command_line_parser&cc)
{


	void* obj = cc.get_exec_node().get_obj();
	//z_status status;
	if (cc.test_char('=') == zs_ok)
	{
		return assignment( cc, cc.get_exec_node());
	}
	if (!(ZFF_SHOW&_flags))
		return zs_access_denied;
	z_string str;
	get_as_string(str, obj);
	cc.get_output_stream() << get_name() << "=" << str << "\n";
	return zs_ok;//???
}
void   zf_var::dump(zf_operation flags, z_stream &stream, void* obj, int tab)
{
	if (!obj)
		return;
	if (!(_flags  &flags))
		return;
	z_string str;
	stream.indent(tab);
	get_as_string(str, obj);
	stream << get_name() << "=" << str << "\n";

}
z_status zf_var::get_as_string(z_string& s, void* vobj)
{
	
	//The vobj may be 0 in case of a cmd parameter
	void* mem_ptr = get_var_ptr(vobj);
	if (!mem_ptr)
		return Z_ERROR(zs_bad_parameter);
	return _var_funcs->get_as_string(s, mem_ptr);
}
z_status zf_var::set_from_string(z_string& s, void* vobj)
{
	//The vobj may be 0 in case of a cmd parameter
	void* mem_ptr = get_var_ptr(vobj);
	if (!mem_ptr)
		return Z_ERROR(zs_bad_parameter);
	return _var_funcs->set_from_string(s, mem_ptr);
}




void zf_var::json_data(z_json_stream& stream, void* obj, int flags)
{
	if (flags&ZFF_JSON_STRUCT)
	{
		stream.keyval("name", zf_feature::get_name());
		stream.keyval("desc", _desc);

		stream.keyval("type", "var");
		stream.key("val");
	}

	z_string val;
	get_as_string(val, obj);
	//z_string esc = z_json_stream::escape(val);
	stream.val(val);
}
/*________________________________________________________________________

zf_prop
________________________________________________________________________*/
zf_prop::zf_prop(z_variable_base* var, ctext id, ctext name, z_memptr offset, zf_operation_flags flags, ctext desc)
	: zf_var(var, id, name, flags, desc)
{
	_offset = offset;
}


/*________________________________________________________________________

zf_param
________________________________________________________________________*/
zf_param::zf_param(z_variable_base* var, ctext id, ctext name, zf_operation_flags flags, ctext desc)
	: zf_var(var, id, name, flags, desc)
{
}







/*________________________________________________________________________

zf_cmd
________________________________________________________________________*/
zf_cmd::zf_cmd(zf_param_list* param_list, ctext id, zf_operation_flags flags, ctext desc)
	: zf_action(id, id, flags, desc)
{
	_params = param_list;
}



/*________________________________________________________________________

zf_child_obj_ptr
________________________________________________________________________*/


// If feature is object, create new one and return pointer
z_void_obj* zf_child_obj_ptr::obj_ptr_create(void* parent) {
	return get_factory()->create_void_obj();
}