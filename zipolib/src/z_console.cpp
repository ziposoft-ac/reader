#include "pch.h"
#include "zipolib/z_filesystem.h"

#include "zipolib/z_console.h"

ZMETA(z_console)
{
	ZBASE(z_factory_controller);


	//ZACT(list_features);

	ZACT(act_exit);
	ZPROP_X(_user_level, "interface_level", ZFF_PROP_DEF, "Interface Level");
	ZPROP_X(_show_duration, "show_time", ZFF_PROP_DEF, "Show elapsed time of command");
	ZPROP_X(_dump_cmd_line,"dump_cmdline",ZFF_PROP_DEF,"Dump the parsed command line contents");
	ZPROP_X(_param_path,"path",ZFF_PROP_DEF,"Current path");
	ZPROP_X(_history,"history", ZFF_LOAD | ZFF_SAVE | ZFF_SHOW,"Command line history");


	ZCMD(runscript, ZFF_CMD_DEF, "run script", ZPRM(z_string, script, "", "Name of script file", ZFF_PARAM));


}
ZMETA(z_console_test_obj)
{
	ZACT(act1);
};
Z_FC_COMMANDS(z_console)
{
	Z_FC_BASE(z_factory_controller);
	ZFC_ACT_X(act_exit,"q",ZFF_ACT_DEF,"Quit/Exit");
	ZFC_ACT(shell);
}
void z_console::init_commands()
{
	Z_FC_INIT(z_console);

}
/*________________________________________________________________________

z_console
________________________________________________________________________*/

z_console::z_console()	  :  z_factory_controller()
{
	_out.add_stdout();
	_show_duration = true;
	_config_file="console.cfg";
	_dump_cmd_line=false;
	z_filesys_getcwd(_startup_path);
	_param_script_step=false;
}

z_console::~z_console()
{
	//_exec_log.close();
}
void z_console::put_prompt()
{
	z_string s;
	get_current_string_path(s);
	zout << s << ">";

	//_obj_current->get_path(_node_path);
	reset_line();
}
z_status  z_console::runapp(int argc, char* argv[],bool autoloadcfg,ctext init_action)
{
	z_status status = zs_ok;


	if (autoloadcfg)
	{
		status=loadcfg();

		if (status)
		{
			if (status != zs_not_found)
			{
				// ignore for now
				//return status;
			}
		}
			
	}

	
	if (init_action)
	{

		status = ExecuteLine(init_action);
		if (status)
			return status;
	}
		

	int i;
	if(argc==1)
	{
		//help();
		ExecuteLine(_param_path);
		run();
	}
	else
	{


		for(i=1;i<argc;i++)
		{
			status = ExecuteLine(argv[i]);
			if (status)
			{
				Z_ERROR_MSG(status, "command \"%s\" failed", argv[i]);
				return status;
			}
		}
	}
	get_current_node_path().get_path_string(_param_path);
	_param_path << "/";
	if(autoloadcfg)
		savecfg();
	return zs_ok;

}
void z_console::OnDoubleBack()
{
	
	get_current_node_path().up();
	zout << "\n";
	put_prompt();


}



void z_console:: OnTab()
{
	try
	{
		if (!_tab_mode)
		{
			//zf_command_context line(this, ZFF_LIST, gz_stdout);
			zf_command_line_parser parser(this);
			zf_node_path path = get_current_node_path();
			z_string partial_string;
			z_status status = parser.evaluate_path(_buffer, path, partial_string);
			/*
			zs_eof - blank line, or bare path ending in slash
			zs_ok - some partial identifier
			zs_unparsed_data - unexpected chars
			zs_skipped - ends in node with no slash*/

			if (status)
			{
				if (status != zs_eof)
					return;
			}
			zf_node& selected = path.get_selected();

			_auto_tab.clear();
			selected.get_feature_strlist(_auto_tab);

			if (selected == get_current_node_path().get_selected())//only autotab the console features if there is no partial path on command line
			{
				get_self().get_feature_strlist(_auto_tab);

			}
			_tab_mode_line_index = get_line_length();

			if ((status == zs_ok) && (partial_string))
			{
				_tab_mode_line_index -= partial_string.size();
				size_t i = 0;
				while (i < _auto_tab.size())
				{
					if (_auto_tab[i].compare(0, partial_string.size(), partial_string))
						_auto_tab.del(i);
					else
						i++;
				}
			}


			_tab_count = _auto_tab.size();
			_tab_mode = true;
			_tab_index = 0;

		}
		if (!_tab_count)
			return;
		trim_line_to(_tab_mode_line_index);
		output(_auto_tab[_tab_index]);
		_tab_index++;
		if (_tab_index >= _tab_count)
			_tab_index = 0;
	}
	catch (z_except & ex)
	{
		ex.dump_to_stream(zout);
	}


}

void z_console::LogInput(ctext text)
{
	//_exec_log<<text<<'\n';


}

z_status z_console::evaluate_feature(zf_node& o, z_string& name)
{
	return Z_ERROR_NOT_IMPLEMENTED;
}

z_status z_console::_ExecuteLineInternal(ctext text)
{

	zf_command_line_parser cmd_line(this, ZFF_ALL);
	z_status status = zs_not_implemented;
	ZT("%s", text);

	z_time time;
	if (_show_duration)
		time.set_now();
	try
	{
		status = cmd_line.evaluate_start(text); //EXECUTE LINE
	}
	catch (z_except & ex)
	{
		ex.dump_to_stream(zout);
		// TODO only if it is a parse error
		//cmd_line.print_context(zout);
	}
	if (_show_duration) {
		_out.format_append("\n%s(%d) %dms\n",zs_get_status_text(status),status, time.get_elapsed_ms());
	}
	return status;

}
z_status z_console::ExecuteLine(ctext text)
{
	z_status status = zs_internal_error;
	try
	{
		status= _ExecuteLineInternal(text);

	}
	catch (z_except & ex)
	{
		ex.dump_to_stream(zout);
	}
	catch (std::exception const&  ex)
	{
		Z_ERROR_MSG(zs_internal_error,
			"Fatal Error: %s", ex.what());
	}
	return status;
}


#define INDENT "  "
#define TAB  '\t'

z_status z_console::shell()
{
	//restore previous path
	ExecuteLine(_param_path);
	run();
	return zs_ok;
}


z_status z_console::get_config_file_path(z_string& path)
{
	path=_startup_path;
	path+='/';	
	path+= _config_file;
	return zs_ok;

}


z_status z_console::runscript(ctext script_name)
{
	z_status status = zs_not_implemented;
//	terminal_open();
	//We have to be careful with act params on recursive functions
	if((!script_name )|| (!strlen(script_name)))
		return Z_ERROR_MSG(zs_bad_parameter,"You must specify a script filename");


	ZTF;
	size_t buff_length;
	char* buffer;
	status = z_file_open_and_read(script_name, &buff_length,&buffer);

	if (status )
		return Z_ERROR_MSG(status, "Could not open file \"%s\"", script_name);
	zf_command_line_parser cmd(this,ZFF_EXE);
	z_string script;
	script.assign(buffer,buff_length);
	z_strlist list;
	script.split('\n',list);
	size_t i;
	for(i=0;i<list.size();i++)
    {
        status = cmd.evaluate_start( list[i]);//SCRIPT

    }
	z_delete(buffer);



	return status;



}



z_status z_console::act_exit()
{
	zout<< "exiting.\n";
	quit();
	return zs_ok;
}
