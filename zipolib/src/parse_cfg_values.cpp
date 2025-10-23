#include "pch.h"
#include "zipolib/parse_cfg_values.h"




ZMETA(zp_cfg_string)
{
	ZPROP(_match);
};
ZMETA(zp_cfg_int)
{
	ZPROP(_val);
};
ZMETA(zp_cfg_float)
{
	ZPROP(_val);
};
ZMETA(zp_cfg_value)
{
};
void zp_test_cfg_quoted_string::build_test()
{

	*this << test<zp_test_quoted_str>("match");
}



#if I_WILL_MAKE_SOMETHING
// TESTS
void zp_test_cfg_assignment::build_test()
{
	*this << ident("name") << skipws() << commit(chr('=')) << skipws() << test<zp_test_group_value>("val");
}


void zp_test_cfg_obj::build_test()
{
	//<< z_new zp_test_obj_contents()
	// << obj<zp_test_obj_contents>("_propname")

	*this << zero_or_more((*z_new zp_test_assignment()) | ';' | (*z_new zp_test_func_call()) | (*z_new zp_test_wsp()));


}



void zp_test_path::build_test()
{
	*this << ident() << skipws() << commit(chr('=')) << skipws() << z_new zp_test_group_value;
}

void zp_test_group_value::build_test()
{

	*this
		<< z_new zp_test_float()//
		<< z_new zp_test_int()
		<< z_new zp_test_ident()
		<< z_new zp_test_quoted_str()
		<< z_new zp_test_single_quoted_str()
		<< z_new zp_test_cfg_obj()
		<< z_new zp_test_obj_map()
		<< z_new zp_test_array_string()
		<< z_new zp_test_array<zp_test_int>()
		<< z_new zp_test_array_empty()
		;


}
void zp_test_array_empty::build_test()
{
	*this << chr('[') << skipws() << ']';
}




void zp_test_cfg_obj::build_test()
{
	//<< z_new zp_test_obj_contents()
	// << obj<zp_test_obj_contents>("_propname")

	*this << optional(commit(chr('<')) << ident("type") << '>') << commit(chr('{'))
		<< test<zp_test_obj_contents>("contents") << '}';
}


void zp_test_obj_contents::build_test()
{

	*this << zero_or_more((*z_new zp_test_assignment()) | ';' | (*z_new zp_test_func_call()) | (*z_new zp_test_wsp()));


}
#endif