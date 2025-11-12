/*
Zipo Library Common Public Header

*/
#ifndef zipo_h
#define zipo_h


/*_________________________________________

Basic defines
___________________________________________*/

#ifdef _DEBUG
#define DEBUG
#endif
#define _CRT_SECURE_NO_WARNINGS 1
#ifdef DEBUG
#define Z_TRACE_ENABLE 1
#endif
//#define __STDC_LIMIT_MACROS
//#define __STDC_CONSTANT_MACROS

#ifdef __cplusplus

#include <cstdint>
#include <string>
#include <cstring>
#include <iostream>
#include <map>
#include <typeinfo>
#include <typeindex>
#include <iostream>
#include <queue>   
#include <fstream>  
#include <sstream>
#include <set>
#include <algorithm>
#include <queue>
#include <stack>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <list>

class z_void_obj
{
    virtual void func() {};
};
#define z_new new
#define z_delete delete

#else
#include <stdint.h>
#include <stdbool.h>

#endif

/*
#include <thread>
#include <cfenv>
#include <condition_variable>
*/
typedef   uint64_t U64;
typedef   int64_t I64;
typedef   uint32_t U32;
typedef   int32_t  I32;
typedef   uint16_t  U16;
typedef   unsigned char  U8;
typedef const char* ctext;
typedef const char* utf8;
/*=====================================================
LINUX
=====================================================*/

#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#define DEBUGBREAK() raise(SIGINT);
typedef int errno_t;




#ifdef DEBUG
#define Z_ASSERT(_X_) { if(!(_X_))  DEBUGBREAK(); }
#else
#define Z_ASSERT(_X_)
#endif

#define zout gz_stdout









#endif
