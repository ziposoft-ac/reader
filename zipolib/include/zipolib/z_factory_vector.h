/*________________________________________________________________________

z_factory_features_h

________________________________________________________________________*/


#ifndef z_factory_vector_h
#define z_factory_vector_h
#include "zipolib/z_factory.h"


/*________________________________________________________________________

zf_child_obj_vector
________________________________________________________________________*/
class zf_child_obj_vector : public  zf_prop, public z_factory
{

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
	zf_child_obj_vector(ctext id, ctext name, z_memptr offset, zf_operation_flags flags, ctext desc = "") :
		zf_prop(&_dummy_var_funcs, id, name, offset, flags, desc), z_factory(name)
	{
		_init = true;
	}
	typedef z_obj_vector<z_void_obj> voidVector;
	voidVector& getVect(void* parent)
	{
		voidVector& map = *(voidVector*)((char*)parent + _offset);
		return map;
	}
	//________________________________________________________________________
	// factory overrides
	virtual z_status get_feature_strlist(z_strlist& list, void* obj, U32 type = zf_ft_all) override;
	virtual  z_status get_child_node(ctext key, zf_node &parent, zf_node &node) override;

	virtual void   dump_features(zf_feature_type type, zf_operation flags, z_stream &stream, void* obj, int tab) override;


	//________________________________________________________________________
	// Feature overrides
	virtual z_status evaluate_cc(zf_command_line_parser &cc) override ;
	virtual z_status assignment_add(z_string name, void* obj, zf_command_line_parser &cc) override;

	virtual z_status cfg_assign(zp_cfg_val* val, z_factory* fact, z_void_obj* obj) override;

	virtual zf_feature_type get_type() override { return zf_ft_obj_list; }
	virtual void   dump(zf_operation flags,z_stream &stream, void* obj, int tab) override;

	// A list is a node
	virtual  z_status get_node(zf_node &parent, zf_node &node);


	virtual z_status get_as_string(z_string& s, void* vobj)
	{
		return zs_not_implemented;
	}

	virtual void dump_object_type(z_stream &stream, void* vobj)
	{

	}
	virtual void json_data(z_json_stream& stream, void* obj,  int flags) override;
	//virtual void json_structure(z_json_stream& stream, void* obj, int flags) override;

	virtual z_void_obj* obj_ptr_get(void* parent) override
	{
		return 0;
	}
	virtual z_status  obj_ptr_set(z_void_obj* parent, z_void_obj* child) override;
	virtual z_void_obj* obj_ptr_create(void* parent) override;
	void delete_all(voidVector& vv) const;


	//________________________________________________________________________
	// overridden by template
	virtual z_void_obj* create_void_obj() override { return 0; }
	virtual void delete_void_obj(void* vo) const override {};

};
template <class TYPE,bool OWNER> class zf_child_obj_vector_t : public zf_child_obj_vector
{
public:
	zf_child_obj_vector_t(ctext id, z_memptr offset) : zf_child_obj_vector(id, id, offset, 0, "")
	{
	}
	virtual void delete_void_obj(void* vo) const override {
		if (OWNER) {
		TYPE* obj = (TYPE*)vo; delete obj;

		}
	}
	virtual z_factory* get_factory() override {
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

template <class VTYPE,bool OWNER>  zf_feature* z_factory::zf_vect_create(z_obj_vector<VTYPE,OWNER>& dummy, ctext id, z_memptr offset, zf_operation_flags flags)
{
	zf_feature* feat = z_new zf_child_obj_vector_t<VTYPE,OWNER>(id, offset);
	feat->set_flags(flags);
	return feat;
}
/*________________________________________________________________________

zf_child_obj_vector_poly

For polymorphic objects. Objects must have virtual function table.
________________________________________________________________________*/
#if 0
class zf_child_obj_vector_poly : public  zf_child_obj_vector
{
public:

};
template <class TYPE> class zf_child_obj_vector_poly_t : public zf_child_obj_vector_poly
{
public:
	zf_child_obj_vector_poly_t(ctext id, z_memptr offset) : zf_child_obj_vector_poly(id, id, offset, 0, "")
	{
	}

	virtual z_factory* get_factory() override {
		return z_factory_t<TYPE>::get_instance_static();
	}

};
#if TODO
template <class VTYPE> inline zf_feature* z_factory::zf_poly_vect_create(z_obj_map<VTYPE>& dummy, ctext id, z_memptr offset, zf_operation_flags flags)
{
	zf_feature* feat = z_new zf_child_obj_vector_poly_t<VTYPE>(id, offset);
	feat->set_flags(flags);
	return feat;
}
#endif
#endif

#endif

