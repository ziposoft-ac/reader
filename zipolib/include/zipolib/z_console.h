#ifndef z_console_h
#define z_console_h
#include "zipolib/z_terminal.h"
#include "zipolib/z_factory_controller.h"
#include "zipolib/z_stream.h"

class z_console_test_obj
{
public:
	int _Aint;
	int othercrap()
	{
		_Aint = 5678;
		return 0;
	}
	virtual z_status act1()
	{
		printf("go running!!!");
		return zs_ok;
	};
};


class z_console : public z_console_base	,public   z_factory_controller
{
protected:

	z_status _ExecuteLineInternal(ctext text);
public:
	virtual zf_node& get_self() 
	{
		if (!_self)
			_self.init((z_void_obj*)this, GET_FACT(z_console));
			return  _self;
	}

	z_stream_multi _out;
	z_console();

	virtual ~z_console();
    virtual void OnDoubleBack();
    z_stream* _exec_log;
/*	
	NEW METHOD
*/
	virtual void LogInput(ctext text);
	virtual z_status evaluate_feature(zf_node& o, z_string& name);
    virtual void OnTab();
	virtual void put_prompt();
	virtual z_status ExecuteLine(ctext text);



	z_status  runapp(int argc, char* argv[],bool loadcfg,ctext init_action=0);

	//command line props
	bool _show_duration;
	bool _dump_cmd_line;
	bool _param_script_step;

	z_status get_config_file_path(z_string& path);


	z_status runscript(ctext script);

	z_status shell();
	z_status act_exit();
	
	virtual void init_commands();
};




#endif








