#include "pch.h"

#include "zipolib/z_factory_controller.h"
#include "zipolib/z_filesystem.h"



ZMETA_DEFV(z_factory_controller);

void z_factory_controller::init_commands()
{
	Z_FC_INIT(z_factory_controller);

}
zf_node& z_factory_controller::get_self()
{
	if (!_self)
		_self.init((z_void_obj*)this, GET_FACT(z_factory_controller));
	return  _self;
}
/*________________________________________________________________________

zf_node_path
________________________________________________________________________*/
void zf_node_path::reset(const zf_node& obj)
{
	clear();
	push_back(obj);
}

z_status zf_node_path::get_path_string(z_string& s)
{
	s = "/";
	uint i;
	for (i = 1; i < size(); i++)
	{
		if (i > 1)
			s << '/';


		s << at(i).get_name();

	}
	return zs_ok;
};
z_status zf_node_path::up()
{
	//z_status status;	
	if (size() > 1)
		pop_back();

	//new_selected = back();

	return zs_ok;
}
/*________________________________________________________________________

zf_command_line
________________________________________________________________________*/



/*________________________________________________________________________

zf_command_context
________________________________________________________________________*/

zf_command_context::zf_command_context(z_factory_controller* fc, zf_node& node, zf_operation oper)
{
	_node = node;
	_operation = oper;
	_fc = fc;
	_pstream =& _fc->get_default_stream();

}
zf_command_context::zf_command_context(z_factory_controller* fc)
{
	_fc = fc;
	_pstream =&( _fc->get_default_stream());
}
zf_node& zf_command_context::get_exec_node()
{
	return _node;
}
z_void_obj* zf_command_context::get_selected_obj()
{
	return _node.get_obj();
}
z_factory*  zf_command_context::get_seclected_factory()
{
	return _node.get_factory();
}

z_json_stream* zf_command_context::get_json_stream()
{
	if (!_json_stream_allocated)
		_json_stream_allocated = z_new z_json_stream(*_pstream);
	return _json_stream_allocated;
}
 z_stream& zf_command_context::get_output_stream()
{
	return *_pstream;
}
z_json_obj & zf_command_context::get_json_params() {
	return _fc->get_json_params();
}


/*

Returns:

zs_eof - blank line, or bare path ending in slash
zs_ok - some partial identifier that is not a node
zs_unparsed_data - unexpected chars
zs_skipped - ends in node with no slash
*/
z_status zf_command_line_parser::evaluate_path(ctext path_str, zf_node_path& path, z_string& feature_str)
{
	set_source(path_str);
	return  _evaluate_path(path, feature_str);
}



/*

Returns:

zs_eof - blank line, or bare path ending in slash
zs_ok -  some partial identifier
zs_unparsed_data - unexpected chars
zs_skipped - ends in node with no slash
zs_child_not_found- child node not found
zs_wrong_object_type- path node is a feature but not an object
*/
z_status zf_command_line_parser::_evaluate_path(zf_node_path& path,  z_string& feature_str)
{
	//ZTF;
	z_status status = zs_ok;
	// evaluate the path
	bool bHavePath = false;
	test_char('$');
	feature_str.clear();
	zf_node node;
	// evaluate the path
	if (test_char('/') == zs_matched)
	{
		path.clear();
		node = _fc->get_root();
		path.push_back(node);
		bHavePath = true;
	}
	else
	{
		node = path.get_selected();
	}
	while (1)
	{
		status = test_any_identifier();
		if (status)
		{
			if (status == zs_no_match)
			{
				//something other than an identifier
				//either starting the line or following a slash
				return zs_unparsed_data;
			}
			if (status == zs_eof)
				return zs_eof;
			return Z_ERROR(status);
		}
		get_match(feature_str);

		//zf_node child;
		if (node.get_child_node(feature_str, node))
		{
			bHavePath = true;
			path.push_back(node);
			feature_str.clear();
			if (test_char('/') == zs_matched)
			{
				zf_command_context cc(_fc, node, ZFF_EXE);
				// This looks really ugly here
				// But this is a callback to each node in the tree as we parse the path
				// to allow the nodes to open any resources, create children, etc.
				zf_action* callback = dynamic_cast<zf_action*>(node.get_feature("on_path_select"));
				if (callback)
					callback->execute(cc);

				continue;
			}
			//next item is child, but no slash
			if (eob())
				// ends in a node with no slash. So don't do autotab
				return zs_skipped;
			// some unexpected chars after node
			return zs_unparsed_data;
		}
		else
			return zs_ok;//we got some partial string which is not a child node
		/*
		if (eob()) //we got some partial string which is not a child node
			return zs_ok;
		//there is still char after
		// this is not an error, we are just done.
		return zs_unparsed_data; 
		*/
	}// end of path evaluation

	return zs_ok;
}

z_status zf_command_line_parser::_evaluate_line(zf_node_path& path, z_string& feature_str)
{

	z_status status = _evaluate_path(path, feature_str);
	if(!feature_str) // good lord this ugly. FIX 
		if (status == zs_unparsed_data)
		{
			// This means we have a node followed by extra chars that is not an identifier
			skip_ws();
			if (test_char(';') == zs_matched)
			{
				return zs_ok; // line is done
			}
			if ((test_string("//") == zs_matched) ||
				(test_char('#') == zs_matched))
			{
				// end of line comment skip to end
				if (test_not_char('\n') == zs_matched)
					return zs_ok; // line is done
				//no end of line? I guess just exit
				return zs_eof;
			}
			if (eob())
				return zs_eof;
			Z_THROW("Unexpected characters");
			return zs_parse_error;
		}
	return status;
}


z_status zf_command_line_parser::evaluate_start(ctext text, uint len)
{
	zf_node_path current_path;

	current_path = _fc->get_current_node_path();

	set_source(text, len);

	if (!_fc)
		Z_THROW("FC not set");

	z_status status = zs_ok;

	// This parses multiple lines!
	while (!eob())
	{
		// Start of new line
		skip_ws();
		if (test_char(';') == zs_matched)
		{
			continue;
		}
		if ((test_string("//") == zs_matched) ||
			(test_char('#') == zs_matched))
		{
			// end of line comment skip to end
			if (test_not_char('\n') == zs_matched)
				continue;
			//no end of line? I guess just exit
			return zs_ok;
		}
		zf_node node;
		zf_node_path line_path = _fc->get_current_node_path();
		z_string feature_str;
		/*

			Returns:

			zs_eof - blank line, or bare path ending in slash
			zs_ok - some partial identifier that is not a node
			zs_unparsed_data - unexpected chars
			zs_skipped - ends in node with no slash
		*/
		status = _evaluate_line(line_path, feature_str);
		node = line_path.get_selected();
		if (feature_str)
		{
			// Test if it is a non-child feature
			zf_feature* feature = node.get_feature(feature_str);
			if (!feature)
			{
				// See if it is a FC feature
				// this is AWFUL
				feature = _fc->get_command(feature_str);
				if (feature)
				{
					//this tells the FC which node to evaluate for built in actions
					_fc->callback_set_temporary_evaluation_node(node); //ugly ugly ugly
					node = _fc->get_self(); //the execution node is now the fc
				}

			}
			if (!feature) //TODO throw parse error
				Z_THROW("'%s' is not a feature of '%s'",
					feature_str.c_str(), node.get_name());


			set_cc(node, ZFF_ALL, _fc->get_default_stream());
			status = feature->evaluate_cc(*this);
			if (status)
				return status;
			continue;
		}
		else
		{
			//we have a node
			if ((status == zs_eof)||(status == zs_skipped))
			{
				current_path = line_path;
				//just a bare node. Select it and exit
				_fc->callback_set_selected_path(line_path);
				if (!eob())
				{
					Z_THROW("Unexpected unparsed chars?");
				}
				return zs_ok;
			}
		}
		Z_THROW("unparsed data");
	}

	return status;

}

/*________________________________________________________________________

z_factory_controller
________________________________________________________________________*/
z_status z_factory_controller::select_obj_from_path(z_string& path)
{


	return Z_ERROR_NOT_IMPLEMENTED;
}



void z_factory_controller::get_current_string_path(z_string &path)
{
	_node_path.get_path_string(path);
}


z_status z_factory_controller::get_node_from_string_path(ctext textpath, zf_node& node)
{
	z_string _out;
	zf_command_line_parser cc(this);
	zf_node_path nodepath;
	z_string feature_str;
	z_status status = cc.evaluate_path(textpath, nodepath, feature_str);
	/*
	zs_eof - blank line, or bare path ending in slash
	zs_ok - some partial identifier
	zs_unparsed_data - unexpected chars
	zs_skipped - ends in node with no slash
	*/
	if ((status != zs_eof)&& (status != zs_skipped))
	{
		if (status) // some other error
			return Z_ERROR(status);
		// some non-node was at the end of the path
		return  Z_ERROR_MSG(zs_child_not_found, "bad node path");
	}



	node = nodepath.get_selected();
	return zs_ok;

}
z_status z_factory_controller::EvaluateTab(ctext text, z_strlist& strlist, z_string& feature_str)
{
#if TODO
	zf_command_context line(this, ZFF_LIST, gz_stdout);
	z_status status = line.evaluate_path(text);

	/*
	zs_eof - blank line, or bare path ending in slash
	zs_ok - some partial identifier
	zs_unparsed_data - unexpected chars
	zs_skipped - ends in node with no slash*/

	if (status)
	{
		if (status != zs_eof)
			return zs_skipped;
	}
	zf_node& selected = line.get_path_node();


	selected.get_feature_strlist(strlist);

	if (selected == get_selected_node())//only autotab the console features if there is no partial path on command line
	{
		get_self().get_feature_strlist(strlist);

	}

	feature_str = line._feature_str;
#endif
	return zs_ok;

}

z_status z_factory_controller::ExecuteLine(ctext text, z_stream& output)
{
	z_status status = zs_not_implemented;
	//_output.clear();
#if TODO

	zf_command_context cmd_line(this, ZFF_ALL, output);

	status = cmd_line.evaluate_start(text); //EXECUTE LINE
	if (status)
	{
		output << cmd_line.error_msg() << "\n";
#ifdef DEBUG
		Z_ERROR_MSG(status,"cmd: %s" ,text);

#endif // DEBUG


	}
#endif
	return status;

}

z_status  z_factory_controller::ExecuteArgs(int argc, char* argv[], z_stream& output)
{
	z_status status = zs_ok;
#if TODO

	zf_command_context cmd(this, ZFF_ALL, output);
	if (argc < 2)
		return zs_bad_parameter;
	status = cmd.evaluate_start(argv[1]); //EXECUTE LINE
#endif
	return zs_ok;

}

z_status z_factory_controller::print_features(zf_node& obj, zf_feature_type type)
{
	z_strlist list;

	obj.get_feature_strlist(list, type);
	for (auto var : list)
	{
		_output << var << "\n";


	}
	return zs_ok;
}

z_status z_factory_controller::list_features_node(zf_node& node, z_stream &output)
{
	z_factory *fact = node.get_factory();
	if (!fact)
		return Z_ERROR_MSG(zs_internal_error,"no factory");
	void *obj = node.get_obj();
	if (!obj)
		return Z_ERROR_MSG(zs_internal_error, "no object");

	//output << "Properties:\n";
	fact->dump_features(zf_ft_prop, ZFF_LIST, output, obj, 1);
	//output << "Actions:\n";
	fact->dump_features(zf_ft_act, ZFF_LIST, output, obj, 1);
	//output << "Stats:\n";
	fact->dump_features(zf_ft_stat, ZFF_LIST, output, obj, 1);



	return zs_ok;
}

z_status z_factory_controller::up()
{

	_node_path.up();
	//_selected_node = _node_path.get_selected();
	return zs_ok;

}
z_status z_factory_controller::help(z_stream &stream)
{
	for (auto i : _fc_commands)
	{
			i.second->dump(ZFF_LIST, stream, 0, 1);
	}
	return zs_ok;


}
z_status z_factory_controller::dump(z_stream &s)
{
	if (!_temporary_evaluation_node)
		Z_THROW_MSG(zs_internal_error, "No eval node");
	_temporary_evaluation_node.dump(zf_ft_all, ZFF_SHOW, s, 0);
	return zs_ok;
}
z_status z_factory_controller::list_features(z_stream &s)
{
	return list_features_node(get_temporary_evaluation_node(), s);

}
z_status z_factory_controller::dumpcfg()
{
	get_root().dump(zf_ft_all, ZFF_SHOW, _output, 0);
	_output << "\n";
	return zs_ok;
}

z_status z_factory_controller::json_data_file()
{
	z_file_out fo("data_dump.json");
	z_json_stream json_str(fo, true);
	get_root().get_factory()->json_data_fact(json_str, get_root().get_obj(), ZFF_JSON_DATA | ZFF_JSON_RECURSE);
	return zs_ok;
}

z_status z_factory_controller::json_data_dump(z_stream &s)
{
	z_json_stream json_str(s, true);
	zf_node& n = get_temporary_evaluation_node();


	n.get_factory()->json_data_fact(json_str, n.get_obj(), ZFF_JSON_DATA | ZFF_JSON_RECURSE);
	return zs_ok;
}
z_status z_factory_controller::json_struct_dump(z_stream &s)
{
	z_json_stream json_str(s, true);
	zf_node& n = get_temporary_evaluation_node();

	//n.get_factory()->json_data_fact(s, n.get_obj(), ZFF_JSON_STRUCT);

	n.get_factory()->json_data_fact(json_str, n.get_obj(), ZFF_JSON_STRUCT);
	//get_selected_node().get_factory()->json_data_fact(json_str, get_selected_node().get_obj(), ZFF_JSON_STRUCT);
	return zs_ok;
}




z_status z_factory_controller::web_context(z_stream &s)
{
	z_string path;
	get_current_string_path(path);
	z_json_stream json(s, true);

	json.obj_start();
	json.keyval("path", path);


	json.key("features");



	get_selected_node().get_factory()->json_data_fact(json, get_selected_node().get_obj(), ZFF_JSON_STRUCT);
	json.obj_end();

	return zs_ok;
}

z_status z_factory_controller::json_struct_file(z_stream &s)
{
	z_file_out fo("struct_dump.json");
	z_json_stream json_str(fo, true);
	get_root().get_factory()->json_data_fact(json_str, get_root().get_obj(), ZFF_JSON_STRUCT | ZFF_JSON_RECURSE);
	return zs_ok;
}



z_status z_factory_controller::get_config_file_path(z_string& path)
{
	path = _startup_path;
	path += '/';
	path += _config_file;
	return zs_ok;

}

z_status z_factory_controller::loadcfg(z_string file_name, z_string nodepath)
{
	z_status status = zs_not_implemented;

	zf_node node;
	if (status = get_node_from_string_path(nodepath, node))
		return status;

	if (file_name == "")
		get_config_file_path(file_name);


	return node.load_cfg(file_name);

}
z_status z_factory_controller::savecfg(z_string file_name, z_string nodepath)
{
	z_status status = zs_not_implemented;

	zf_node node;
	if (status = get_node_from_string_path(nodepath, node))
		return status;


	if (file_name == "")
		get_config_file_path(file_name);



	return node.save_cfg(file_name);
}


void z_factory_controller::add_command(ctext key, zf_action * feature)
{
	if (_fc_commands.exists(key))
	{
		Z_THROW_MSG(zs_already_exists, "Command \"%s\" already exists!", key);
	}
	_fc_commands[key] = feature;
}

void z_factory_controller::init_config_file_name(ctext exe_name,ctext root_name)
{
	z_string full = exe_name;
	z_string base_name;
	z_filesys_get_path_parts(full, 0, &base_name, 0);
	z_filesys_getcwd(_startup_path);
	_config_file = base_name + "." + root_name + ".cfg";


}