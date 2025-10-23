#pragma once
/*
Zipo Library Common Public Header

*/
#ifndef zipo_lib_h
#define zipo_lib_h

#include "zipolib/zipo.h"
#include "zipolib/z_status.h"
#include "zipolib/z_string.h"
#include "zipolib/z_exception.h"
#include "zipolib/z_error.h"
#include "zipolib/z_map.h"
#include "zipolib/z_strlist.h"
#include "zipolib/z_log.h"
#include "zipolib/z_time.h"
#include "zipolib/z_file.h"


bool z_debug_load_save_args(int* pargc, char*** pargv);
typedef void (*z_catch_ctl_c_handler_t) (int s);
z_status z_catch_ctl_c(z_catch_ctl_c_handler_t handler);


class zipo_library
{
	//internal arg list to replace args in debug
	int _argc;
	char** _argv;
	//storage for debug args
	z_strlist _arg_list;
	size_t _debug_log_file;

public:
	zipo_library();
	~zipo_library();

	bool debugReplay() {
		return _argv != 0;
	}
	

	bool debug_load_save_args(int* pargc, char*** pargv);
};





#endif
