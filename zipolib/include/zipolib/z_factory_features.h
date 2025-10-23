/*________________________________________________________________________

z_factory_features_h

________________________________________________________________________*/


#ifndef z_factory_features_h
#define z_factory_features_h
#include "zipolib/zipolib.h"
#include "zipolib/z_factory_node.h"
#include "zipolib/z_obj_list.h"
//#include "zipolib/parse.h"


class zf_command_line_parser;
class zp_cfg_val;

/*________________________________________________________________________

zf_feature
________________________________________________________________________*/
class zf_feature
{
	z_string _name;
protected:
	zf_operation_flags _flags = 0;

public:
	z_string _desc;
	zf_feature();
	zf_feature(ctext id, ctext name, zf_operation_flags flags, ctext desc = "");
	virtual ~zf_feature() {}

	virtual zf_feature_type get_type() { return zf_ft_none; }
	virtual ctext  get_name() { return _name; }
	virtual ctext  get_desc() { return _desc; }
	virtual z_status add_to_list(z_strlist& list, void* obj) { list << _name; return zs_ok; }

	virtual z_status cfg_assign(zp_cfg_val* val, z_factory* fact, z_void_obj* obj) { return Z_ERROR_NOT_IMPLEMENTED; }


	virtual z_status assignment( zf_command_line_parser&cc, zf_node &node) { return Z_ERROR_NOT_IMPLEMENTED; }
	virtual z_status evaluate_cc(zf_command_line_parser&cc);
	virtual z_status evaluate(void* vobj);
	virtual  z_status get_node(zf_node &parent, zf_node &node) { return zs_child_not_found; }


	virtual void   list(zf_operation_flags flags, z_stream &stream, void* obj) {   };
	virtual void   dump(zf_operation_flags flags, z_stream &stream, void* obj, int tab) {   };
	virtual void   save(z_stream &stream, void* obj, int tab) {   };


	// If feature is object, return pointer
	virtual z_void_obj* obj_ptr_get(void* parent) {
		return 0;
	}
	virtual z_status  obj_ptr_set(z_void_obj* parent, z_void_obj* child) {
		 return Z_ERROR_NOT_IMPLEMENTED; 
	}
	// If feature is object, create new one and return pointer
	virtual z_void_obj* obj_ptr_create(void* parent) {
		return 0;
	}
	virtual void json_data(z_json_stream& stream, void* obj,  int flags) {  }
	virtual void json_structure(z_json_stream& stream, void* obj,  int flags) { }

	zf_user_level get_user_level();
	void set_user_level(zf_user_level lvl);
	zf_operation_flags get_flags() {
		return _flags;
	}
	bool test_flag(zf_operation_flags flag) {
		return (0 != (_flags&flag)) ;
	}
	void set_flags(zf_operation_flags flags) {
		_flags = flags;
	}

};
/*________________________________________________________________________

zf_stat
________________________________________________________________________*/
class zf_stat : public  zf_feature
{
public:
	zf_stat(ctext id, ctext name, zf_operation_flags flags, ctext desc) : zf_feature(id, name, flags, desc)
	{

	}
	virtual ~zf_stat() {}
	virtual zf_feature_type get_type() { return zf_ft_stat; }
	static const zf_feature_type get_catagory_static() { return zf_ft_stat; }
	virtual z_status evaluate_cc(zf_command_line_parser& cc);

	virtual z_status get_as_string(z_string& s, void* obj) = 0;
	virtual void   dump(zf_operation flags, z_stream &stream, void* obj, int tab);
	virtual void json_data(z_json_stream& stream, void* obj, int flags) override;
	//virtual void json_structure(z_json_stream& stream, void* obj,  int flags) override;

};
template <class SUBCLASS, class BASECLASS, class VAL_TYPE> class zf_stat_t : public zf_stat
{
public:
	typedef VAL_TYPE(BASECLASS::*FUNC)();
private:

	FUNC _func;
	z_variable<VAL_TYPE> _var_funcs;
public:
	virtual z_status get_as_string(z_string& s, void* obj)
	{

		SUBCLASS*  cobj = reinterpret_cast<SUBCLASS*>(obj);

		
		VAL_TYPE v = (cobj->*_func)();
		return _var_funcs.get_as_string(s, (void*)&v);

	}


	zf_stat_t(FUNC func, ctext id, zf_operation_flags flags, ctext desc) : zf_stat(id, id, flags, desc)
	{
		_func = func;
	}
	virtual ~zf_stat_t() {}

};


/*________________________________________________________________________

zf_static_string
________________________________________________________________________*/
class zf_const_string : public zf_feature
{
protected:

	ctext _const_string;
public:


	zf_const_string(ctext str, ctext id, ctext name, zf_operation_flags flags, ctext desc) : zf_feature(id, name, flags, desc)
	{
		_const_string = str;
	}

	virtual ~zf_const_string() {}


	virtual ctext get() { return  _const_string; }


};



/*________________________________________________________________________

zf_var
________________________________________________________________________*/
class zf_var : public zf_feature
{
protected:

	z_variable_base* _var_funcs;
public:


	zf_var(z_variable_base* var, ctext id, ctext name, zf_operation_flags flags, ctext desc);
	virtual ~zf_var() {}

	virtual void* get_var_ptr(void* obj) = 0;

	virtual z_status get_as_string(z_string& s, void* v);
	virtual z_status set_from_string(z_string& s, void* vobj);

	virtual z_status evaluate_cc(zf_command_line_parser&cc);
	virtual void   dump(zf_operation flags, z_stream &stream, void* obj, int tab);
	virtual z_status assignment( zf_command_line_parser&cc, zf_node &node);
	virtual void json_data(z_json_stream& stream, void* obj, int flags);
};




/*________________________________________________________________________

zf_prop
________________________________________________________________________*/



class zf_prop : public zf_var
{
protected:
	z_memptr _offset;

public:
	virtual void* get_var_ptr(void* obj)
	{
		void* mem_ptr = (char*)obj + _offset;
		return mem_ptr;
	}
	static const zf_feature_type get_catagory_static() { return zf_ft_mvar; }
	virtual zf_feature_type get_type() { return zf_ft_mvar; }
	virtual z_factory* get_factory() { return 0; }
	virtual z_status cfg_assign(zp_cfg_val* val, z_factory* fact, z_void_obj* obj) override;

	zf_prop(z_variable_base* var, ctext id, ctext name, z_memptr offset, zf_operation_flags flags, ctext desc);
	virtual ~zf_prop() {}

};
/*________________________________________________________________________

zf_param
________________________________________________________________________*/
class zf_param : public zf_var
{
protected:
public:



	static const zf_feature_type get_catagory_static() { return zf_ft_param; }
	virtual zf_feature_type get_type() { return zf_ft_param; }

	zf_param(z_variable_base* var, ctext id, ctext name, zf_operation_flags flags, ctext desc);
	virtual ~zf_param() {}


};
class zf_param_list : public z_obj_list<zf_param, true>
{
public:

    void add(zf_param* param);
	zf_param* get_param_by_name(const z_string& name)
	{
		for (zf_param* p : *this)
		{
			if (name == p->get_name())
				return p;
		}
		return 0;
	}

};
template <class TYPE> class zf_param_t : public zf_param
{
	z_variable<TYPE> _var_funcs=z_variable<TYPE>();

	TYPE _var=TYPE();
public:
	zf_param_t(ctext id, zf_operation_flags flags, ctext desc = "") : zf_param(&_var_funcs, id, id, flags, desc)
	{
	}
	virtual ~zf_param_t() {}
	virtual void* get_var_ptr(void* obj)
	{
		// params return a direct pointer to the stand alone variable
		return &_var;

	}
	TYPE& get_data_ref() { return  _var; }
};



template <class TYPE> class zf_prop_t : public zf_prop
{
	z_variable<TYPE> _var_funcs;
public:
	zf_prop_t(ctext id, z_memptr offset, zf_operation_flags flags, ctext desc = "") : zf_prop(&_var_funcs, id, id, offset, flags, desc)
	{
	}
	virtual ~zf_prop_t() {}

};


/*________________________________________________________________________

zf_action_base
________________________________________________________________________*/
class zf_action : public  zf_feature
{

	//z_obj_vector<zf_prop> _params;
protected:
	zf_param_list* _params;

public:
	zf_action(ctext id, ctext name, zf_operation_flags flags, ctext desc = "")
		: zf_feature(id, name, flags, desc)
	{
		_params = 0;
	}
	virtual zf_feature_type get_type() { return zf_ft_act; }
	static const zf_feature_type get_catagory_static() { return zf_ft_act; }
	virtual ~zf_action() {}
	//virtual void display(z_file& f, void* obj);

	//todo refactor this crap. too convoluted.
	//the cc should not call the feature and then the feature call back to the cc
	zf_param_list* get_param_list() { return _params; };
	virtual z_status evaluate_cc(zf_command_line_parser& cc);
	virtual z_status load_params(zf_command_line_parser& cc);
	virtual z_status execute(zf_command_context &cc) = 0;
	virtual void   dump(zf_operation flags, z_stream &stream, void* obj, int tab);
	virtual z_status assignment( zf_command_line_parser&cc, zf_node &node)
	{
		return load_params(cc);
	}
	virtual void json_data(z_json_stream& stream, void* obj, int flags) override;
	

	//virtual void json_structure(z_json_stream& stream, void* obj,  int flags) override;
};
/*________________________________________________________________________

zf_cmd
________________________________________________________________________*/
class zf_cmd : public  zf_action
{




public:
	zf_cmd(zf_param_list* param_list, ctext id, zf_operation_flags flags, ctext desc = "");
	virtual ~zf_cmd()
	{
		if (_params)
			delete _params;
	}

};

template <class OBJT, class MFUNC> class zf_cmds_t : public zf_cmd
{
	MFUNC _mfunc;
public:
	zf_cmds_t(zf_param_list* list, MFUNC mfunc, ctext id, zf_operation_flags flags, ctext desc)
		: _mfunc(mfunc), zf_cmd(list, id, flags, desc)
	{
	}
	virtual ~zf_cmds_t() {}

	virtual z_status execute(zf_command_context &cc);


};
template <class OBJT, class MFUNC> class zf_cmd_t : public zf_cmd
{
	MFUNC _mfunc;
public:
	zf_cmd_t(zf_param_list* list, MFUNC mfunc, ctext id, zf_operation_flags flags, ctext desc)
		: _mfunc(mfunc), zf_cmd(list, id, flags, desc)
	{
	}
	virtual ~zf_cmd_t() {}

	virtual z_status execute(zf_command_context &cc);


};
/*
No used yet?? 

*/
template <class OBJT, class FACT_FUNC> class zf_fact_cmd_t : public zf_cmd
{
	FACT_FUNC _mfunc;
public:
	zf_fact_cmd_t(zf_param_list* list, FACT_FUNC mfunc, ctext id, zf_operation_flags flags, ctext desc)
		: _mfunc(mfunc), zf_cmd(list, id, flags, desc)
	{
	}
	virtual ~zf_fact_cmd_t() {}

	virtual z_status execute(zf_command_context &cc);

};


/*________________________________________________________________________

zf_action
________________________________________________________________________*/
class zf_mfunc : public  zf_action
{

	z_ptr_member_func _offset=0 ;


public:
	zf_mfunc(ctext id, ctext name, z_ptr_member_func offset, zf_operation_flags flags, ctext desc = "");
	virtual ~zf_mfunc() {}
	// TODO-do we even want this functionality anymore? This is using props as "params" for memeber funcs that have no params
	//z_obj_vector<zf_prop> _params;
	//virtual void display(z_file& f, void* obj);
	virtual z_status execute(zf_command_context &cc);


};
class zf_mfunc_stream : public  zf_action
{
	z_ptr_member_func_stream _offset=0;
public:
	zf_mfunc_stream(ctext id, ctext name, z_ptr_member_func_stream offset, zf_operation_flags flags, ctext desc = "");
	virtual ~zf_mfunc_stream() {}
	virtual z_status execute(zf_command_context &cc);


};
class zf_mfunc_json : public  zf_action
{
	z_ptr_member_func_json _offset;
public:
	zf_mfunc_json(ctext id, ctext name, z_ptr_member_func_json offset, zf_operation_flags flags, ctext desc = "");
	virtual ~zf_mfunc_json() {}
	virtual z_status execute(zf_command_context& cc);
	virtual z_status execute(z_json_stream & stream,z_factory* fact,z_void_obj* obj,z_json_obj& params);
	virtual z_status load_params(zf_command_line_parser &cc);
};

/*________________________________________________________________________

zf_child_obj
________________________________________________________________________*/
class zf_child_obj_base : public  zf_prop
{

protected:
public:
	//	zf_prop(z_variable_base* var,ctext id, ctext name, z_memptr offset, zf_operation_flags flags, ctext desc);

	zf_child_obj_base(z_variable_base* var, ctext id, ctext name, z_memptr offset, zf_operation_flags flags, ctext desc) :
		zf_prop(var, id, name, offset, flags, desc)
	{
	}

	virtual ~zf_child_obj_base() {}


	virtual z_status evaluate_cc(zf_command_line_parser&cc);

	virtual zf_feature_type get_type() { return zf_ft_obj; }
	static const zf_feature_type get_catagory_static() { return zf_ft_obj; }

	virtual void   dump(zf_operation flags, z_stream &stream, void* obj, int tab);
	// A list is a node
	virtual  z_status get_node(zf_node &parent, zf_node &node);

	virtual void json_data(z_json_stream& stream, void* obj, int flags);
	//virtual void json_structure(z_json_stream& stream, void* obj,  int flags) override;
	// TODO move this to base class??
	virtual z_factory* get_factory_from_vobj(z_void_obj* vo);
	virtual z_status get_feature_strlist(z_strlist& list, void* obj);


	virtual z_status cfg_assign(zp_cfg_val* val, z_factory* fact, z_void_obj* obj) override;

};




/*________________________________________________________________________

zf_child_obj_ptr
________________________________________________________________________*/
class zf_child_obj_ptr : public  zf_child_obj_base
{

protected:
	z_variable_object_ptr _var;
public:
	zf_child_obj_ptr(ctext id, ctext name, z_memptr offset, zf_operation_flags flags, ctext desc) :
		zf_child_obj_base(&_var, id, name, offset, flags, desc)
	{
	}

	virtual ~zf_child_obj_ptr() {}
	virtual z_void_obj* obj_ptr_get(void* parent) override
	{
		if (!parent)
			return 0;
		size_t* mem_ptr_ptr = (size_t*)get_var_ptr(parent);
		size_t mem_ptr_val = *mem_ptr_ptr;

		void* mem_ptr = (void*)(mem_ptr_val);
		return (z_void_obj*)mem_ptr;
	}
	void assign_ptr(void* parent, void* ptr)
	{
		size_t* mem_ptr_ptr = (size_t*)get_var_ptr(parent);

		*mem_ptr_ptr = (size_t)ptr;
	}
	virtual z_status  obj_ptr_set(z_void_obj* parent, z_void_obj* child) override {
		assign_ptr(parent, child);
		return zs_ok;
	}
	// If feature is object, create new one and return pointer
	virtual z_void_obj* obj_ptr_create(void* parent) override;
	virtual z_factory* get_factory_from_vobj(z_void_obj* vo);


};
template <class TYPE> class zf_child_obj_ptr_t : public zf_child_obj_ptr
{
public:
	zf_child_obj_ptr_t(ctext id, ctext name, z_memptr offset, zf_operation_flags flags, ctext desc)
		: zf_child_obj_ptr(id, name, offset, flags, desc)
	{
	}
	virtual ~zf_child_obj_ptr_t() {}
	virtual z_factory* get_factory();

};
/*________________________________________________________________________

zf_child_obj
________________________________________________________________________*/
class zf_child_obj : public  zf_child_obj_base
{

protected:
	z_variable_object _var;

public:
	zf_child_obj(ctext id, ctext name, z_memptr offset, zf_operation_flags flags, ctext desc) :
		zf_child_obj_base(&_var, id, name, offset, flags, desc)
	{
	}

	virtual ~zf_child_obj() {}
	virtual z_void_obj* obj_ptr_get(void* parent)
	{
		if (!parent)
			return 0;
		return (z_void_obj*)get_var_ptr(parent);
	}


};

template <class TYPE> class zf_child_obj_t : public zf_child_obj
{
public:
	zf_child_obj_t(ctext id, ctext name, z_memptr offset, zf_operation_flags flags, ctext desc)
		: zf_child_obj(id, name, offset, flags, desc)
	{
	}
	virtual ~zf_child_obj_t() {}
	virtual z_status get_as_string(z_string& s, void* vobj)
	{

		return zs_not_implemented;

	}
	virtual z_factory* get_factory();

};


#define RECAST(_TYPE_,_NAME_) _TYPE_& _NAME_= *reinterpret_cast<_TYPE_*>(v);



class zf_feature_list : public z_obj_map< zf_feature>
{
public:
	virtual ~zf_feature_list()
	{
	}
};




#endif

