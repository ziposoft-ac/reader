#ifndef z_factory_node_h
#define z_factory_node_h
#include "zipolib/zipolib.h"
#include "zipolib/z_stream_json.h"
#include "zipolib/z_map.h"
#include "zipolib/z_vector.h"
#include "zipolib/z_parse_text.h"
#include "zipolib/z_variable.h"


class z_json_obj;
class z_generic_class_base {};
class z_generic_class_base2 {};
class z_generic_class : public z_generic_class_base, z_generic_class_base2 {};
typedef z_status(z_generic_class::*z_ptr_member_func)();
typedef z_status(z_generic_class::*z_ptr_member_func_stream)(z_stream& stream);
typedef z_status(z_generic_class::*z_ptr_member_func_json)(z_json_stream& stream, z_json_obj& params);


typedef unsigned long long z_memptr;

#if 0
// Operations
const int ZFF_LOAD=	0x00000001; // Load from file
const int  ZFF_SAVE=0x00000002;// Save from file
const int ZFF_LIST=0x00000004; // List
const int ZFF_SET=0x00000008; // allow set
const int ZFF_EXE=0x00000010; // Allow exec?
const int ZFF_WRITE=0x00000020; // Allow write (set,load)

const int ZFF_SHOW=0x00000080 ;// Parameter
union zf_operation_flags_struct
{
    U64 as64;
    struct {
        U64 load :1;
        U64 save :1;
        U64 list :1;
        U64 set :1;
        U64 exe :1;
        U64 write :1;
    } bits;
};
#endif
typedef  U64 zf_operation_flags;

typedef  U64 zf_operation;

// 

enum zf_user_level
{
	zf_ul_anon,
	zf_ul_user,
	zf_ul_admin,
	zf_ul_debug,

};

enum zf_feature_type
{
	zf_ft_none,
	zf_ft_mvar=1,
	zf_ft_param=2,
	zf_ft_obj=4,


	//Actions



	// Obj lists:
	zf_ft_obj_list=8,

	zf_ft_stat = 0x10, // read only func returns value
	zf_ft_act = 0x20,
	/*
	zf_ft_mfunc = 0x20, // member function()
	zf_ft_cmd = 0x40, // 
	zf_ft_act = zf_ft_cmd | zf_ft_mfunc,
	*/
	zf_ft_obj_ptr = 0x80, // 

	zf_ft_prop = zf_ft_mvar | zf_ft_obj | zf_ft_obj_list| zf_ft_obj_ptr,
	zf_ft_all=0xfff,

};

//Formating flags
#define ZF_JSON_PRETTY    	0x00000001
#define ZF_JSON_RECURSE    	0x00000002

// Operations
#define ZFF_LOAD		0x00000001 // Load from file
#define ZFF_SAVE		0x00000002 // Save from file
#define ZFF_LIST		0x00000004 // List 
#define ZFF_SET	        0x00000008 // allow set
#define ZFF_EXE 		0x00000010 // Allow exec?
#define ZFF_WRITE 		0x00000020 // Allow exec?

#define ZFF_SHOW  	    0x00000080 // Parameter


#define ZFF_JSON_STRUCT		0x00001000 // ??
#define ZFF_JSON_DATA		0x00002000 // 
#define ZFF_JSON_RECURSE	0x00004000 // 
#define ZFF_JSON_ARRAY_DATA	0x00008000 // 
#define ZFF_JSON_ARRAY_COLS	0x00010000 // 

#define ZFF_ALL    	    0xFFFFFFFF

#define ZFF_USER_LEVEL_MASK 0x0F000000
#define ZFF_UL_USER         0x01000000

extern ctext ZFF_PARSE_STRING;

#define ZFF_HEX			0x01000000
#define ZFF_CMD_DEF  (ZFF_LOAD|ZFF_SAVE|ZFF_LIST|ZFF_SET|ZFF_SHOW|ZFF_JSON_STRUCT)
#define ZFF_PROP_DEF (ZFF_LOAD|ZFF_SAVE|ZFF_LIST|ZFF_SET|ZFF_SHOW|ZFF_JSON_DATA|ZFF_JSON_STRUCT|ZFF_WRITE)
#define ZFF_READ_ONLY (ZFF_LIST|ZFF_SHOW|ZFF_JSON_DATA|ZFF_JSON_STRUCT)
#define ZFF_PROP_NOLOAD (ZFF_SHOW|ZFF_LIST|ZFF_SET)
#define ZFF_PARAM (ZFF_LOAD|ZFF_SAVE|ZFF_SET|ZFF_WRITE)
//#define ZFF_STAT_DEF (ZFF_SAVE|ZFF_LIST|ZFF_SHOW)
// Dont save the stats for now.
#define ZFF_STAT_DEF (ZFF_LIST|ZFF_SHOW|ZFF_JSON_DATA|ZFF_JSON_STRUCT)
#define ZFF_DYN   (ZFF_LIST|ZFF_SET)
#define ZFF_CONST  (ZFF_LIST|ZFF_SHOW)

#define ZFF_ACT_DEF   (ZFF_LIST|ZFF_EXE|ZFF_JSON_STRUCT)


#define zp_offsetof_class(_class_,_member_)   (size_t)&reinterpret_cast<const volatile char&>((((_class_*)0)->_member_))


/*________________________________________________________________________

zf_node
________________________________________________________________________*/
class z_factory;
class zf_mfunc;
class zf_action;
class zf_feature_list;
class zf_feature;
class zf_command_context;
class zf_command_line_parser;

class z_factory_controller;


class zf_node
{
	z_void_obj* _obj;
	z_factory* _factory;// either a factory or a feature
	z_string _name; // for dynamic nodes, used by child obj lists
	//otherwise name is the class name


public:
	zf_node()
	{
		init(0, 0);

	}
	zf_node(z_void_obj* o, z_factory* ft)
	{
		init(o, ft);

	}
	zf_node(const z_void_obj* o);
	void init(z_void_obj* o, z_factory* ft);
	void set(const z_void_obj* o);

	z_void_obj* get_obj() { return _obj; }
	z_factory* get_factory() { return _factory; }

	bool get_child_node(ctext key, zf_node& node);
	virtual  zf_feature* get_feature(ctext key);
	operator bool()
	{
		return _obj != 0;
	}
	virtual ctext get_name();
	virtual void set_name(ctext name) { _name = name; }
	z_status get_feature_strlist(z_strlist& list,U32 type= zf_ft_all);
	//virtual z_status execute_act_ptr(z_ptr_member_func act_addr);
	virtual z_status execute(zf_command_context &cc);
	virtual z_status assignment( zf_command_line_parser& cc);
	virtual z_status assignment_add(z_string name, zf_command_line_parser&cc);

	virtual void dump(zf_feature_type type, zf_operation_flags oper, z_stream& stream, int tab);

	virtual z_status save_cfg(ctext file_name);
	virtual z_status load_cfg(ctext file_name);
};


const zf_node zf_node_null;

#endif
