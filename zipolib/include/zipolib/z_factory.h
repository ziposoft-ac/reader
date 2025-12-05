/*________________________________________________________________________

z_factory_h

________________________________________________________________________*/


#ifndef z_factory_h
#define z_factory_h
#include "zipolib/zipolib.h"
#include "zipolib/z_string.h"
#include "zipolib/z_factory_features.h"
#include "zipolib/z_factory_macros.h"
#include "zipolib/z_strlist.h"



//________________________________________________________________________
// z_factory 
//
class z_factory
{
	ctext _name;
	zf_feature_list _features;
protected:
	bool _init;

	//z_factory* _base_factory;
public:

	z_factory(ctext name);

	virtual ~z_factory()
	{

	}

	//________________________________________________________________________
	// overridden by template
	virtual std::type_index get_type_index() = 0;
	virtual z_status execute_act_ptr(void* vobj, z_ptr_member_func act_addr);
	virtual z_status execute_act_stream_ptr(void* vobj, z_ptr_member_func_stream act_addr, z_stream& stream);;
	virtual z_status execute_act_json_ptr(void* vobj, z_ptr_member_func_json act_addr, z_json_stream& stream, z_json_obj& params);;
	virtual void init_factory() {};
	virtual z_void_obj* create_void_obj() = 0;
	virtual z_void_obj* create_null_obj() { return 0; };
	virtual void delete_void_obj(void* vo)  const = 0;
	//virtual zp_test* get_parse_test(zp_test* parent) {return 0;	}

	//________________________________________________________________________
	// optional overrides



	//________________________________________________________________________
	// 
	//
	virtual  zf_feature* get_feature(ctext key);
	virtual  zf_feature* get_feature_idx(size_t idx);
	virtual  z_status get_child_node(ctext key, zf_node &parent, zf_node &node);

	virtual z_status get_feature_strlist(z_strlist& list, void* obj, U32 type = zf_ft_all);

	virtual z_status assignment_add( z_string name, void* obj, zf_command_line_parser& cc) { return zs_operation_not_supported; }
	virtual z_status assignment( zf_command_line_parser& cc, void* obj);
	//virtual z_status parse_assign( zp_result_obj_contents* cc, void* obj);


	virtual void dump_obj( void* obj)
	{

		dump_features(zf_ft_prop, ZFF_SHOW, gz_stdout, obj, 0);
	}
	
	virtual void json_array(z_json_stream& stream, void* obj, int flags);
	virtual void json_data_fact(z_json_stream& stream, void* obj, int flags);
	virtual void json_structure(z_json_stream& stream, void* obj,  int flags);

	virtual void dump_features(zf_feature_type type, zf_operation oper, z_stream& stream, void* obj, int tab);
	virtual void save(z_stream& stream, void* obj, int tab);
	z_status file_load(ctext filename, z_void_obj * obj);


	void check_init()
	{
		if (!_init)
		{
			_init = true;
			init_factory();
		}
	}

	

	virtual const char* get_name() const { return _name; };
	zf_feature* add_feature(ctext key, zf_feature* feature);

	template <class FTYPE> z_status get_features_type(z_obj_vector<FTYPE,false>& list)
	{
		for (auto i : _features)
		{
			FTYPE* p = is_category<FTYPE>(i.second);
			if (p)
				list.add(p);
		}
		if (list.size())
			return zs_not_found;
		return zs_ok;
	}
	template <class FTYPE> FTYPE* is_category(zf_feature* f)
	{
		if (!f) return 0;
		if (f->get_type() & FTYPE::get_category_static())
		{
			FTYPE* act = static_cast<FTYPE*>(f);
			return act;
		}
		return 0;
	}
	template <class FTYPE> FTYPE* get_feature_t(ctext key)
	{
		zf_feature* f = get_feature(key);
		return dynamic_cast<FTYPE*>(f);
	}
	template <class FTYPE> FTYPE* get_feature_category(ctext key)
	{
		zf_feature* f = get_feature(key);
		return is_category<FTYPE>(f);
	}

	//________________________________________________________________________
	// ACTIONS
	//
	//zf_action* add_act_params(ctext id, ctext name, z_ptr_member_func act_addr, zf_operation_flags flags, ctext desc, int num_params, ...);
	zf_action* add_mfunc(ctext id, ctext name, z_ptr_member_func act_addr, zf_operation_flags flags, ctext desc)
	{
		zf_action* action = z_new	zf_mfunc(id, name, *(z_ptr_member_func*)&act_addr, flags, desc);
		add_feature(name, action);
		return action;
	}
	zf_action* add_mfunc_stream(ctext id, ctext name, z_ptr_member_func_stream act_addr, zf_operation_flags flags, ctext desc)
	{
		zf_action* action = z_new	zf_mfunc_stream(id, name, *(z_ptr_member_func_stream*)&act_addr, flags, desc);
		add_feature(name, action);
		return action;
	}
	zf_action* add_mfunc_json(ctext id, ctext name, z_ptr_member_func_json act_addr, zf_operation_flags flags, ctext desc)
	{
		zf_action* action = z_new	zf_mfunc_json(id, name, *(z_ptr_member_func_json*)&act_addr, flags, desc);
		add_feature(name, action);
		return action;
	}
	//________________________________________________________________________
	// STATS
	//



	//________________________________________________________________________
	// LIST
	//
	template <class VTYPE,bool OWNER>  zf_feature* zf_vect_create(z_obj_vector<VTYPE,OWNER>& dummy, ctext id, z_memptr offset, zf_operation_flags flags);
	template <class VTYPE, bool OWNER> zf_feature* zf_map_create(z_obj_map<VTYPE,  OWNER>& dummy, ctext id, z_memptr offset, zf_operation_flags flags);
	template <class VTYPE>  zf_feature* zf_poly_map_create(z_obj_map<VTYPE>& dummy, ctext id, z_memptr offset, zf_operation_flags flags);
	//________________________________________________________________________
	// OBJS
	//
	template <class VTYPE>  zf_feature* zf_child_obj_create(VTYPE& dummy, ctext id, z_memptr offset, zf_operation_flags flags, ctext desc)
	{
		zf_feature* f = z_new zf_child_obj_t<VTYPE>(id, id, offset,flags,desc);
		//f->_desc = desc;
		//f->set_flags(flags);
		return f;
	}
	//________________________________________________________________________
	// CMD 
	//
	template <class OBJT, class MFUNC>  zf_cmd_t<OBJT, MFUNC>* cmd_add(zf_param_list* params, MFUNC mfunc, ctext id, zf_operation_flags flags, ctext desc)
	{
		zf_cmd_t<OBJT, MFUNC>* cmd = z_new zf_cmd_t<OBJT, MFUNC>(params, mfunc, id, flags, desc);
		add_feature(id, cmd);

		return cmd;
	}
	template <class OBJT, class MFUNC>  zf_cmds_t<OBJT, MFUNC>* cmds_add(zf_param_list* params, MFUNC mfunc, ctext id, zf_operation_flags flags, ctext desc)
	{
		zf_cmds_t<OBJT, MFUNC>* cmd = z_new zf_cmds_t<OBJT, MFUNC>(params, mfunc, id, flags, desc);
		add_feature(id, cmd);

		return cmd;
	}
	template <class VTYPE, class DTYPE> inline VTYPE& param_add(zf_param_list* list, ctext id, DTYPE def_value, ctext desc, zf_operation_flags flags)
	{
		if (list->get_param_by_name(id))
		{
			Z_ERROR_MSG(zs_already_exists, "Duplicate Parameter: %s", id);
		}

		zf_param_t<VTYPE>* param;
		param = z_new zf_param_t<VTYPE>(id, flags, desc);

		list->add(param);
		param->get_data_ref() = def_value;


		return param->get_data_ref();
	}
	//________________________________________________________________________
	// FACT_CMD 
	//
	template <class OBJT, class MFUNC>  zf_fact_cmd_t<OBJT, MFUNC>* fact_cmd_add(zf_param_list* params, MFUNC mfunc, ctext id, zf_operation_flags flags, ctext desc)
	{
		zf_fact_cmd_t<OBJT, MFUNC>* cmd = z_new zf_fact_cmd_t<OBJT, MFUNC>(params, mfunc, id, flags, desc);
		add_feature(id, cmd);

		return cmd;
	}

	//________________________________________________________________________
	// PROPS
	//

	// For creating pointer properties
	template <class VTYPE> inline zf_feature* zf_prop_add(VTYPE* &dummy, ctext id, z_memptr offset, zf_operation_flags flags, ctext desc)
	{
		z_string name = id;

		name.trim_leading_underscore();

		zf_feature* feature = get_feature(name);
		if (!feature)
		{
			feature = z_new zf_child_obj_ptr_t<VTYPE>(name, name, offset, flags, desc);
			_features[name] = feature;
		}

		return feature;
	}
	template <class VTYPE> inline zf_feature* zf_prop_add(VTYPE& dummy, ctext id, z_memptr offset, zf_operation_flags flags, ctext desc)
	{
		z_string name = id;

		name.trim_leading_underscore();

		zf_feature* feature = get_feature(name);
		if (!feature)
		{
			feature = z_new zf_prop_t<VTYPE>(name, offset, flags, desc);
			_features[name] = feature;
		}

		return feature;
	}




};


class z_factory_map : public  z_obj_map<z_factory,false>
{
	std::unordered_map<std::type_index, z_factory*> _type_map;
public:
	void shutdown()
	{
		delete_all();
	}
	z_factory* get_factory(ctext name);
	z_factory* get_factory_by_type(std::type_index ti);
	z_factory* get_factory_from_vobj(const z_void_obj* vo);
	void add_factory(ctext name, z_factory* fact);


};
class z_factory_map_def : public z_factory_map
{
public:

};
template <class LIST_NAME>  z_factory_map* get_factory_list_t()
{
	static LIST_NAME theList;
	return &theList;
}
z_factory_map* get_factory_list();
z_factory* get_factory_by_name(ctext name);


template <class CTYPE > class z_factory_t : public z_factory
{
		static z_factory_t<CTYPE> _fact_obj;// defined by macro
	
public:
	typedef  CTYPE THECLASS;
	z_factory_t(ctext name) : z_factory(name)
	{
		get_factory_list()->add_factory(name, this);
	}


	//________________________________________________________________________
	// static  
	static z_factory_t<CTYPE>* get_instance_static()
	{
		_fact_obj.check_init();
		return &_fact_obj;
	}


	//________________________________________________________________________
	// The implementation of these are constructed by the macros
	template<class BASECLASS> static void static_add_members_class(z_factory *f);
	static CTYPE* static_create_new_obj();


	//________________________________________________________________________
	// overrides 

	virtual z_void_obj* create_null_obj() override
	{
		CTYPE* obj = 0;
		return (z_void_obj*)obj;
	}

	virtual z_void_obj* create_void_obj() override
	{ 
		return (z_void_obj*)static_create_new_obj();
	}
	virtual void delete_void_obj(void* vo) const override  { CTYPE* obj = (CTYPE*)vo; delete obj; }
	virtual void init_factory() { static_add_members_class<CTYPE>(this); };

	virtual z_status execute_act_ptr(void* vobj, z_ptr_member_func act_addr) override
	{
		typedef z_status(CTYPE::*funcptr)();
		CTYPE*  cobj = reinterpret_cast<CTYPE*>(vobj);
		void* pp = &act_addr;
		funcptr fp = *(funcptr*)(pp);
		return (cobj->*fp)();
	}
	
	virtual z_status execute_act_stream_ptr(void* vobj, z_ptr_member_func_stream act_addr,z_stream& stream) override
	{
		typedef z_status(CTYPE::*funcptr)(z_stream& s);
		CTYPE*  cobj = reinterpret_cast<CTYPE*>(vobj);
		// convert to pointer to func pointer
		void* pp = &act_addr;
		// member func conversion magic
		funcptr fp = *(funcptr*)(pp);
		return (cobj->*fp)(stream);
	};

	virtual z_status execute_act_json_ptr(void* vobj, z_ptr_member_func_json act_addr, z_json_stream& stream, z_json_obj& params) override
	{
		typedef z_status(CTYPE::*funcptr)(z_json_stream& s, z_json_obj& params);
		CTYPE*  cobj = reinterpret_cast<CTYPE*>(vobj);
		// convert to pointer to func pointer
		z_ptr_member_func_json* pp = &act_addr;
		// member func conversion magic
		void* vpp=(void*)pp;

		funcptr fp = *(funcptr*)(vpp);
		return (cobj->*fp)(stream, params);
	};
	virtual std::type_index get_type_index() override  { 	return std::type_index(typeid(CTYPE)); 	}
	//virtual zp_test* get_parse_test(zp_test* parent) override  { return 0; }


	//________________________________________________________________________
	// STATS
	//
	template <class SUBCLASS, class BASECLASS, class VAL_TYPE> inline static zf_stat* zf_stat_add(VAL_TYPE(BASECLASS::*get_func)(), ctext id, zf_operation_flags flags, ctext desc)
	{
		zf_stat* stat = z_new zf_stat_t<SUBCLASS, BASECLASS, VAL_TYPE>(get_func, id, flags, desc);
		return stat;
	}


	
};
template<class TYPE>  z_factory* zf_child_obj_t<TYPE>::get_factory() {
	return z_factory_t<TYPE>::get_instance_static();
}
template<class TYPE>  z_factory* zf_child_obj_ptr_t<TYPE>::get_factory() {
	return z_factory_t<TYPE>::get_instance_static();
}

template <class VTYPE> inline  zf_node create_node(VTYPE & _obj_)
{
	
	return zf_node((z_void_obj*)&_obj_, z_factory_t<VTYPE>::get_instance_static());
}

z_factory* get_factory_from_vobj(const z_void_obj* vo);

#define GET_FACT(_CLASS_) z_factory_t<_CLASS_>::get_instance_static()



template <class PTYPE,class CTYPE> z_status get_child_objs_type(z_factory* pf,PTYPE* parent,z_obj_map<CTYPE,false>& map)
{
	z_obj_vector<zf_child_obj_ptr,false> children;
	pf->get_features_type<zf_child_obj_ptr>(children);

	for (auto cp : children)
	{
		auto cf=cp->get_factory();
		if (!cf) continue;
		auto classf=GET_FACT(CTYPE);
		if (cf!=classf) {
			continue;

		}
		auto vo=cp->obj_ptr_get(parent);
		CTYPE* co=reinterpret_cast<CTYPE*>(vo);
		if (!co) continue;
		map.add(cp->get_name(),co);


}
	if (map.size())
		return zs_not_found;
	return zs_ok;
}

#endif

