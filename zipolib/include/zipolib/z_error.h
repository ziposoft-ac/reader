#ifndef z_error_h
#define z_error_h
#include "zipolib/zipo.h"
#include "zipolib/z_log.h"



/*
errors can be reported many ways.

To stdout, to the zipo log, to the OS debug/trace facilities (DbgPrint), to custom callbacks, etc.


*/

#if 1 

#define	Z_ERROR(status)  get_zlog().error_t (status,z_log::lvl_error,__FILE__,__FUNCTION__,__LINE__)
#define	Z_WARN(status)  get_zlog().error_t (status,z_log::lvl_warning,__FILE__,__FUNCTION__,__LINE__)
#define	Z_ERROR_MSG(status,...)  get_zlog().error_msg_t (status,z_log::lvl_error,__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define	Z_WARN_MSG(status,...)  get_zlog().error_msg_t (status,z_log::lvl_error,__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define	Z_ERROR_NOT_IMPLEMENTED   get_zlog().error_t (zs_operation_not_supported,z_log::lvl_error,__FILE__,__FUNCTION__,__LINE__)

#else
#define	Z_ERROR_MSG(status,...)  (status)
#define	Z_ERROR(status)  (status)
#define	Z_ERROR_NOT_IMPLEMENTED  (zs_operation_not_supported)

#endif


#if 0
z_status z_get_os_error(z_string *msg=0);



#define	Z_ERROR_DBG(status)   z_logger_get().add_msg (z_logger_lvl_debug,__FILE__,__FUNCTION__,__LINE__,status,0);

#define	Z_LOG_ERROR_MSG(...)    z_logger_get().add_msg (z_logger_lvl_error,__FILE__,__FUNCTION__,__LINE__,zs_ok,__VA_ARGS__);

//#define Z_ERROR_THROW(_error_no_,_error_msg_) {zb_error* e=z_new  zb_error(_error_no_);e->_error_msg.Format _error_msg_; throw e;}
#endif
#endif

