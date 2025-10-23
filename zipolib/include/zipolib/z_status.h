#ifndef z_status_h
#define z_status_h

#include "zipolib/zipo.h"



#ifdef	__cplusplus
extern "C" {
#endif

typedef enum  {
	zs_ok=0,
	zs_matched=0,
	zs_success=0,
	zs_no_match,
	zs_skipped,
	zs_unparsed_data,
	zs_eof,
	zs_end_of_list,
	zs_child_not_found,
	zs_fatal_error,//fatal errors below. Status above!
	zs_syntax_error,
	zs_unknown_error,
	zs_could_not_open_dir,
	zs_could_not_create_dir,
	zs_could_not_open_file,
	zs_read_error,
	zs_write_error,
	zs_not_open,
	zs_already_open,
	zs_bad_parameter,
	zs_item_not_found,
 	zs_feature_not_found,
	zs_operation_not_supported,
	zs_invalid_conversion,
	zs_not_implemented,
	zs_cannot_create_virtual_obj,
	zs_no_entry_for_item,
	zs_io_error,
	zs_template_syntax_error,
	zs_tmpl_expected_closing_parenthesis,
	zs_internal_error,
	zs_wrong_object_type,
	zs_out_of_range,
	zs_data_error,
	zs_parse_error,
	zs_already_exists,
	zs_not_found,
	zs_access_denied,
	zs_timeout,
	zs_too_long,
	zs_out_of_memory,
    zs_quit,
    zs_bad_command,
    zs_failed_on_device,
} z_status;
ctext zs_get_status_text(z_status status);

z_status z_get_os_error();

#ifdef	__cplusplus
} //END "C" functions
#endif

#endif

