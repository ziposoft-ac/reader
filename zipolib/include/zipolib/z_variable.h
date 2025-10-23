/*________________________________________________________________________

 z_variable_h

________________________________________________________________________*/


#ifndef z_variable_h
#define z_variable_h
#include "zipolib/z_parse_text.h"

z_status from_string(const z_string &s, z_strlist& type);


template <class T> z_status from_string(const z_string &s, T& type)
{
	std::stringstream instream(std::ios::in ); 
	instream.str(s);
	instream >> type;
	return zs_ok;
}


class z_variable_base
{
public:
   /*
   If v==0 then parser is advanced, but assignment skipped.
   */
	virtual z_status load_from_parser(zp_text_parser &parser, void* v) const { return Z_ERROR_NOT_IMPLEMENTED; }
	virtual z_status set_from_string(z_string& s, void* v) const 
	{ 
		return Z_ERROR_NOT_IMPLEMENTED; 
	};
	virtual z_status set_from_int(I64& s, void* v) const { return Z_ERROR_NOT_IMPLEMENTED; };
	virtual z_status set_from_float(double& s, void* v) const { return Z_ERROR_NOT_IMPLEMENTED; };
	virtual z_status get_as_string(z_string& s, void* v) const { return Z_ERROR_NOT_IMPLEMENTED; };
	//virtual z_status set_from_parse_vector(zp_result_vector* cc, void* v) const  { return Z_ERROR_NOT_IMPLEMENTED; };

};
class z_variable_object : public z_variable_base
{


};
class z_variable_object_ptr : public z_variable_base
{


};

template <class VAR >  class z_variable : public z_variable_base
{
public:
	virtual z_status load_from_parser(zp_text_parser &parser, void* v) const;
	virtual z_status set_from_string(z_string& s, void* v) const;
	virtual z_status get_as_string(z_string& s, void* v) const;
	//virtual z_status set_from_parse_result(zp_result* cc, void* v) const;
	virtual z_status set_from_int(I64& s, void* v) const;
	virtual z_status set_from_float(double& s, void* v) const;
	//virtual z_status set_from_parse_vector(zp_result_vector* cc, void* v) const;

};











#endif

