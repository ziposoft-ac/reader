/*________________________________________________________________________

z_filesystem.h

________________________________________________________________________*/

#ifndef z_filesystem_h
#define z_filesystem_h
#ifdef	__cplusplus
extern "C" {
#endif
#include "zipolib/zipo.h"

#define Z_MAX_PATH_LENGTH 2048

typedef size_t z_file_h;
typedef size_t z_directory_h;
z_status  z_file_open_and_read(utf8 in_filepath, size_t * bytesread,char** pbuffer );
z_status z_file_delete(utf8 name);
z_status z_file_exists(utf8 name);

z_status z_filesys_get_current_dir(char* dir,int length);

z_status z_directory_change(utf8 dir,int create);
z_status z_directory_delete(utf8 dir);
z_status z_directory_delete_tree(utf8 name);

z_status z_dir_create(utf8 dir_name,int ignore_if_exists);

//utf8 z_get_filename_from_path(utf8 fullpath);

z_status z_dir_open(utf8 name,z_directory_h* h);
void z_dir_close(z_directory_h h);

#define Z_DIR_TYPE_DIR  1
#define Z_DIR_TYPE_FILE 2
z_status z_dir_get_next(z_directory_h h,utf8* currentfile,int requestedtypes,int *typeout);


#if 0  //UNUSED
z_status z_file_open_and_write(utf8 in_filepath,U8* data,unsigned long length  );
z_status z_fopen(z_file_h* filep,utf8 _Filename,ascii _Mode);
#endif

#ifdef	__cplusplus
}
#endif
#endif

