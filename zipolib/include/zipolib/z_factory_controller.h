/*________________________________________________________________________

z_factory_h

________________________________________________________________________*/


#ifndef z_factory_controller_h
#define z_factory_controller_h

#include "zipolib/z_factory_cc.h"
#include "zipolib/z_factory_map.h"
#include "zipolib/z_factory_vector.h"
#include "zipolib/parse_json.h"
#include "zipolib/z_filesystem.h"







/*________________________________________________________________________

zf_node_path
________________________________________________________________________*/
class zf_node_path : public std::vector<zf_node>
{
public:
	zf_node_path() {}
	zf_node_path(zf_node& node) {
		push_back(node);
	}
	z_status get_path_string(z_string& s);
	z_status up();
	void reset(const zf_node& obj);
	zf_node& get_selected() { 
		if (size() == 0)
			Z_THROW("Node path is empty");
		return back(); }

};

class z_factory_controller;

class z_factory_handler
{

public:



};


/*________________________________________________________________________

zf_command_context
________________________________________________________________________*/
class zf_command_line_parser : public  zp_text_parser, public  zf_command_context
{
	// This is the temporary path for the current executing line
	//zf_node_path _path_line;

	z_status _evaluate_path(zf_node_path& path, z_string& feature);
	z_status _evaluate_line(zf_node_path& path, z_string& feature);
public:
	zf_command_line_parser(z_factory_controller* fc, zf_operation oper) : zf_command_context(fc) {
		_operation = oper;

	}
	zf_command_line_parser(z_factory_controller* fc) : zf_command_context(fc){
	}

	
	z_status evaluate_path(ctext path_str, zf_node_path& path, z_string& feature_str);
	z_status evaluate_start(ctext text, uint len = -1);

};

class z_factory_controller_base
{
public:
	virtual z_status ExecuteLine(ctext text,  z_stream& output) = 0;
	virtual z_status EvaluateTab(ctext text, z_strlist& strlist, z_string& feature_str) = 0;
	virtual void get_current_string_path(z_string &path) = 0;

};


class z_factory_controller : public z_factory_controller_base
{
	friend zf_command_context;
	friend zf_command_line_parser;
	
	zf_node_path _node_path;




protected:
	zf_node _root;
	z_stream& _output;

	//this is initialized by the subclasses
	zf_node _self;

	zf_node _temporary_evaluation_node;

	z_obj_map< zf_action> _fc_commands;
	zf_action* get_command(ctext key)
	{
		return _fc_commands.getobj(key);
	}



	z_json_obj _json_params;
	z_status list_features_node(zf_node& obj, z_stream &s);
	z_status list_props_node(zf_node& obj, z_stream &s);
	z_status list_acts_node(zf_node& obj, z_stream &s);
	z_status print_features(zf_node& obj, zf_feature_type type);


	void set_path(const zf_node_path& path) { _node_path = path; }


	virtual void callback_set_temporary_evaluation_node(zf_node& node) { _temporary_evaluation_node = node; }
	
	

	virtual void callback_set_selected_path(zf_node_path& path) { _node_path = path; }

 	z_status select_obj_from_path(z_string& path);

	z_status get_config_file_path(z_string& path);
public:
	z_factory_controller() : _output(gz_stdout)
	{
	}	

	virtual z_status EvaluateTab(ctext text, z_strlist& strlist, z_string& feature_str) override;
	virtual z_status ExecuteLine(ctext text,  z_stream& output);
	void add_command(ctext key, zf_action * feature);

	zf_node_path& get_current_node_path() {
		return _node_path;
	}
	z_status ExecuteArgs(int argc, char* argv[],  z_stream& output);
	//Called from load/save config to specify node
	z_status get_node_from_string_path(ctext path, zf_node& node);
	/*
	*/
	virtual void init_commands();
	virtual void init_config_file_name(ctext exe_name, ctext root_name);
	template <class ROOT_OBJ_CLASS> void initialize(ROOT_OBJ_CLASS * obj /* root object */,ctext exe_name /* exe/config file name */)
	{

		z_factory* fact = GET_FACT(ROOT_OBJ_CLASS);
		init_config_file_name(exe_name, fact->get_name());
		_root.init((z_void_obj*)obj, fact);
		_node_path.reset(_root);
		init_commands();
	}

	virtual void get_current_string_path(z_string &path);
	z_stream& get_default_stream() {
		return _output;
	}
	zf_node& get_selected_node() {
		return _node_path.back();
	}
	// The temporary_evaluation_node is the tree node the FC is executing one of its acts on
	zf_node& get_temporary_evaluation_node() {
		return _temporary_evaluation_node;
	}

	//Root of the tree
	zf_node& get_root() { return  _root; }

	virtual zf_node& get_self();

	//props (read only)
	z_string _config_file;
	z_string _startup_path;

	//command line actions
	z_status list_features(z_stream &s);
	z_status list_props(z_stream &s);
	z_status list_acts(z_stream &s);
	z_status help(z_stream &s);
	z_status dump(z_stream &s);
	z_status up();
	z_status dumpcfg();
	z_status json_data_file();
	z_status web_context(z_stream &s);
	z_status json_data_dump(z_stream &s);
	z_status json_struct_dump(z_stream &s);
	z_status json_struct_file(z_stream &s);
	z_status savecfg(z_string file_name = "", z_string node = "/");
	z_status loadcfg(z_string file_name = "", z_string node = "/");

	z_json_obj & get_json_params() {return _json_params;}


	//template<class FC_CLASS> static void static_init_commands(z_factory_controller *fc);

};

// TODO - clean this up !
#define ZFC_ACTS_X(_ACT_,_NAME_,_FLAGS_,_DESC_) {typedef z_status (FC_SUB::*fn_act)(z_stream& s); \
	fn_act _func_##_ACT_=&FC_SUB::_ACT_;\
	fc->add_command(_NAME_,z_new zf_mfunc_stream(#_ACT_,_NAME_,*(z_ptr_member_func_stream*)(&_func_##_ACT_) ,_FLAGS_,_DESC_));}
#define ZFC_ACTS(_ACT_) ZFC_ACTS_X(_ACT_,#_ACT_,ZFF_ACT_DEF,"")

#define ZFC_ACT_X(_ACT_,_NAME_,_FLAGS_,_DESC_) {typedef z_status (FC_SUB::*fn_act)(); \
	fn_act _func_##_ACT_=&FC_SUB::_ACT_;\
	fc->add_command(_NAME_,z_new zf_mfunc(#_ACT_,_NAME_,*(z_ptr_member_func*)(&_func_##_ACT_) ,_FLAGS_,_DESC_));}
#define ZFC_ACT(_ACT_) ZFC_ACT_X(_ACT_,#_ACT_,ZFF_ACT_DEF,"")

#define ZFC_ACT_JSON_X(_ACT_,_NAME_,_FLAGS_,_DESC_) {typedef z_status (FC_SUB::*mem_fn_type)(z_json_stream& s,z_json_obj& obj); \
	mem_fn_type fn=&FC_SUB::_ACT_; \
	fc->add_command(_NAME_,z_new zf_mfunc_json(#_ACT_,_NAME_,*(z_ptr_member_func_json*)(&fn) ,_FLAGS_,_DESC_));};

#define ZFC_ACT_JSON(_ACT_) ZFC_ACT_JSON_X(_ACT_,#_ACT_,ZFF_ACT_DEF,"")


ZMETA_DECL(z_factory_controller)
{
#if 0

	ZACT(dumpcfg);
	ZACT(json_data_file);

	ZACT_X(up, "up", ZFF_ACT_DEF, "Go up a level");

#endif
	ZCMD(savecfg, ZFF_CMD_DEF, "save config",
		ZPRM(z_string, file_name, "", "file name. Defaults to exec name.cfg", ZFF_PARAM),
		ZPRM(z_string, node, "/", "node to save, defaults to root", ZFF_PARAM));
	ZCMD(loadcfg, ZFF_CMD_DEF, "load config",
		ZPRM(z_string, file_name, "", "file name. Defaults to exec name.cfg", ZFF_PARAM),
		ZPRM(z_string, node, "/", "node to load, defaults to root", ZFF_PARAM));
};


#define Z_FC_COMMANDS(FC_BASE)  template<class FC_SUB> void FC_BASE##static_init_commands(z_factory_controller *fc)
#define Z_FC_BASE(FC_BASE) FC_BASE##static_init_commands<FC_SUB>(fc);

#define Z_FC_INIT(FC_CLASS) FC_CLASS##static_init_commands<FC_CLASS>(this);
Z_FC_COMMANDS(z_factory_controller)
{
	ZFC_ACTS(help);
	ZFC_ACTS(json_struct_file);
	ZFC_ACTS(json_struct_dump);
	ZFC_ACTS(json_data_dump);
	ZFC_ACTS(web_context);
	ZFC_ACTS(dump);
	ZFC_ACTS_X(list_features, "ls", ZFF_ACT_DEF, "List features");
	ZFC_ACTS_X(list_props, "lp", ZFF_ACT_DEF, "List props");
	ZFC_ACTS_X(list_acts, "la", ZFF_ACT_DEF, "List actions");

}

#endif

