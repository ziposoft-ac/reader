/*________________________________________________________________________

z_factory_map_

________________________________________________________________________*/


#ifndef z_factory_map_h
#define z_factory_map_h
#include "zipolib/z_factory.h"


/*________________________________________________________________________

zf_child_obj_map
________________________________________________________________________*/
class zf_child_obj_map : public  zf_prop, public z_factory
{

	int _dummy_member = 123;
protected:


	zf_node get_list_obj(zf_node& parent)
	{
		z_void_obj* membervar = (z_void_obj*)((char*)parent.get_obj() + _offset);
		zf_node obj(membervar, this);
		return obj;

	}

	z_status _addObj(ctext name)
	{
		// TODO check if polymorphic?
		return zs_not_implemented; 

	}

	z_variable_base _dummy_var_funcs;
public:

	zf_child_obj_map(ctext id, ctext name, z_memptr offset, zf_operation_flags flags, ctext desc = "");

	//________________________________________________________________________
	// factory overrides
	virtual z_status get_feature_strlist(z_strlist& list, void* obj, U32 type = zf_ft_all);
	virtual  z_status get_child_node(ctext key, zf_node &parent, zf_node &node);

	virtual void   dump_features(zf_feature_type type, zf_operation flags, z_stream &stream, void* obj, int tab);
	virtual void json_data_fact(z_json_stream& stream, void* obj, int flags) override;
	
	//________________________________________________________________________
	// actions

	z_status data_table_json(z_void_obj* map, z_stream &stream);


	//________________________________________________________________________
	// Feature overrides
	virtual z_status evaluate_cc(zf_command_line_parser &cc) ;
	virtual z_status assignment_add(z_string name, void* obj,zf_command_line_parser &cc);
	virtual z_status cfg_assign(zp_cfg_val* val, z_factory* fact, z_void_obj* obj) override;
	virtual zf_feature_type get_type()  { return zf_ft_obj_list; }
	virtual void   dump(zf_operation flags,z_stream &stream, void* obj, int tab) ;

	// A list is a node
	virtual  z_status get_node(zf_node &parent, zf_node &node);


	virtual z_status get_as_string(z_string& s, void* vobj)
	{
		return zs_not_implemented;
	}

	virtual void dump_object_type(z_stream &stream, void* vobj)
	{

	}

	virtual z_void_obj* obj_ptr_get(void* parent) override
	{
		return 0;
	}
	virtual z_status  obj_ptr_set(z_void_obj* parent, z_void_obj* child) override;
	virtual z_void_obj* obj_ptr_create(void* parent) override;
	virtual void json_data(z_json_stream& stream, void* obj, int flags);
	virtual void json_data_children(z_json_stream& stream, void* obj,  int flags);
	//virtual void json_structure(z_json_stream& stream, void* obj,  int flags) override;
	//________________________________________________________________________
	// overridden by template
	virtual z_void_obj* create_void_obj() override { return 0; }
	virtual void delete_void_obj(void* vo) const override {};

	//________________________________________________________________________
	voidMap& getMap(void* parent)
	{
		z_void_obj* vobj=(z_void_obj*)((char*)parent + _offset);
		voidMap* vmap = reinterpret_cast<voidMap*>(vobj);
		//voidMap* vmap = dynamic_cast<voidMap*>(vobj);

		voidMap& map = *(vmap);
		return map;
	}

};
template <class TYPE> class zf_child_obj_map_t : public zf_child_obj_map
{
public:
	zf_child_obj_map_t(ctext id, z_memptr offset) : zf_child_obj_map(id, id, offset, 0, "")
	{
		if (_desc == "")
		{
			z_factory* fact = get_factory();
			if(fact)
			_desc << "Map of " << fact->get_name() << " objects";
		}
	}

	virtual z_factory* get_factory() override{
		return z_factory_t<TYPE>::get_instance_static();
	}
	virtual void dump_object_type(z_stream &stream, void* v)
	{
		z_void_obj* vobj = (z_void_obj*) v;
		z_factory* fact=get_factory_list()->get_factory_from_vobj(vobj);
		if (fact)
		{
			stream << '<' << fact->get_name() << '>';

		}
		else
		{
			//TODO 
			stream << "<unknown>";

		}
	}
	virtual std::type_index get_type_index()
	{
		return std::type_index(typeid(TYPE));

	}
};

template <class VTYPE, bool OWNER> inline zf_feature* z_factory::zf_map_create(z_obj_map<VTYPE, OWNER>& dummy, ctext id, z_memptr offset, zf_operation_flags flags)
{
	z_obj_map<VTYPE, OWNER>* map = &dummy;
	offset = (size_t)map;

	zf_feature* feat = z_new zf_child_obj_map_t<VTYPE>(id, offset);
	feat->set_flags(flags);
	return feat;
}

#if 0
/*________________________________________________________________________

zf_child_obj_map_poly

For polymorphic objects. Objects must have virtual function table.
________________________________________________________________________*/
class zf_child_obj_map_poly : public  zf_child_obj_map
{
public:

};
template <class TYPE> class zf_child_obj_map_poly_t : public zf_child_obj_map_poly
{
public:
	zf_child_obj_map_poly_t(ctext id, z_memptr offset) : zf_child_obj_map_poly(id, id, offset, 0, "")
	{
	}

	virtual z_factory* get_factory() override {
		return z_factory_t<TYPE>::get_instance_static();
	}

};

template <class VTYPE> inline zf_feature* z_factory::zf_poly_map_create(z_obj_map<VTYPE>& dummy, ctext id, z_memptr offset, zf_operation_flags flags)
{
	zf_feature* feat = z_new zf_child_obj_map_poly_t<VTYPE>(id, offset);
	feat->set_flags(flags);
	return feat;
}



void get_json_from_vmap(z_json_stream& stream, z_factory* default_fact, voidMap& vmap, int flags);
#endif
#endif

