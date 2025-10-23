/*________________________________________________________________________

z_factory_h

________________________________________________________________________*/


#ifndef z_factory_cc_h
#define z_factory_cc_h

#include "zipolib/z_factory_node.h"
#include "zipolib/z_factory_features.h"




class z_factory_controller;



/*________________________________________________________________________

zf_command_context
________________________________________________________________________*/
class zf_command_context
{
public:

private:

	// TODO Clean this up
	z_json_stream* _json_stream_allocated = 0;
	z_stream* _pstream = &gz_stream_null;

protected:
	z_factory_controller* _fc=0;
	zf_operation _operation= ZFF_LIST;
	zf_node _node;
public:

	zf_command_context(z_factory_controller* fc, zf_node& node, zf_operation oper);
	zf_command_context(z_factory_controller* fc);
	virtual ~zf_command_context()
	{
		//TODO - this is AWFUL
		if (_json_stream_allocated)
			delete _json_stream_allocated;
	}
	void set_cc(zf_node& exec_node, zf_operation oper, z_stream& stream)
	{
		_node = exec_node;
		_operation = oper;
		_pstream = &stream;
	}
	zf_command_context(zf_node& node, zf_operation oper, z_stream& output) {}
	//zf_command_context(z_factory_controller* fc, zf_operation oper,z_stream& output);

	z_void_obj* get_selected_obj();
	z_factory*  get_seclected_factory();
	zf_node& get_exec_node();
	virtual z_json_stream* get_json_stream();
	z_stream& get_output_stream();

	z_json_obj & get_json_params();
	zf_operation& operation() {
		return _operation;
			
	}


};



template <class OBJT, class FACT_FUNC> z_status zf_fact_cmd_t<OBJT, FACT_FUNC>::execute(zf_command_context &cc) {


	OBJT* fact = dynamic_cast<OBJT*>(cc.get_seclected_factory());
	z_void_obj* pobj = cc.get_selected_obj();
	
	if (!fact)
		return zs_internal_error;
	z_status status = _mfunc(*fact,pobj,cc.get_output_stream());
	return status;
}

template <class OBJT, class MFUNC> z_status zf_cmd_t<OBJT, MFUNC>::execute(zf_command_context &cc) {
	OBJT* pobj = reinterpret_cast<OBJT*>(cc.get_selected_obj());
	if (!pobj)
		return zs_internal_error;
	z_status status = _mfunc(*pobj);
	return status;
}
template <class OBJT, class MFUNC> z_status zf_cmds_t<OBJT, MFUNC>::execute(zf_command_context &cc) {
	OBJT* pobj = reinterpret_cast<OBJT*>(cc.get_selected_obj());
	if (!pobj)
		return zs_internal_error;
	z_status status = _mfunc(*pobj, cc.get_output_stream());
	return status;
}


#endif

