#ifndef z_error_h
#define z_error_h
#include "zipolib/zipo.h"
#include "zipolib/z_log.h"



/*
errors can be reported many ways.

To stdout, to the zipo log, to the OS debug/trace facilities (DbgPrint), to custom callbacks, etc.


*/

#if 1 
#define	Z_ERROR_MSG(status,...)  z_log_error_msg_t (status,__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)
#define	Z_WARN_MSG(status,...)  z_log_warn_msg_t (status,__FILE__,__FUNCTION__,__LINE__,__VA_ARGS__)

#define	Z_ERROR(status)  z_log_error_t(status,__FILE__,__FUNCTION__,__LINE__)
#define	Z_WARN(status)  z_log_warn_t (status,__FILE__,__FUNCTION__,__LINE__)

#define	Z_ERROR_NOT_IMPLEMENTED  z_log_error_t (zs_operation_not_supported,__FILE__,__FUNCTION__,__LINE__)

#else
#define	Z_ERROR_MSG(status,...)  (status)
#define	Z_ERROR(status)  (status)
#define	Z_ERROR_NOT_IMPLEMENTED  (zs_operation_not_supported)

#endif
#ifdef DEBUG
#define	Z_DBG_WARN_RETURN(status)  z_debug_warn_t(status,__FILE__,__FUNCTION__,__LINE__)
#else
#define	Z_DBG_WARN_RETURN(status)  (status)

#endif

#endif

