/*________________________________________________________________________

z_os_specific.h

________________________________________________________________________*/

#include "zipolib/zipo.h"
#ifndef z_os_specific_h
#define z_os_specific_h

#ifdef WINDOWS
#ifdef	__cplusplus
extern "C" {
#endif

U32 PrintWin32Error ();
BOOL AnsiToUnicode16(utf8 in_Src, WCHAR *out_Dst, INT in_MaxLen);
BOOL AnsiToUnicode16L(utf8 in_Src, INT in_SrcLen, WCHAR *out_Dst, INT in_MaxLen);
int WCHAR_str_deallocate(WCHAR* in);
WCHAR* WCHAR_str_allocate(utf8 in_Src,size_t MaxLen);

char* UTF8_str_allocate(WCHAR* in_src);
int UTF8_str_deallocate(char* in_src);
BOOL Unicode16ToAnsi(WCHAR * in_Src, char* out_Dst, INT in_MaxLen);
BOOL Unicode16ToUtf8(WCHAR * in_Src, char* out_Dst, INT in_MaxLen);


#ifdef	__cplusplus
}
//z_status  get_win32_error (z_string);


#endif
#endif

#endif //WINDOWS
