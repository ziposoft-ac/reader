#include "pch.h"
#include "zipolib/zipolib.h"
#include "zipolib/z_directory.h"
#include "zipolib/z_os_specific.h"

//#include <filesystem>



#include <errno.h>
#ifdef BUILD_VSTUDIO
#include "direct.h"
//#include "Shlwapi.h"
extern "C" {
BOOL __stdcall  PathFileExistsW( LPCWSTR pszPath);
}
#else
#include  <string.h>
#include  <errno.h>
#define MAX_PATH 2048
#include  <unistd.h>
#include  <dirent.h>
#include  <sys/stat.h>
#define __cdecl 

#define _chdir chdir

#endif
#if UNIX
#endif

/*

*/
z_status __cdecl z_file_exists(utf8 in_filepath)
{
#ifdef BUILD_VSTUDIO

	
	WCHAR* w_filepath=WCHAR_str_allocate(in_filepath,Z_MAX_PATH_LENGTH);

	BOOL b=PathFileExistsW(	 w_filepath);

	WCHAR_str_deallocate(w_filepath);
	return (b? zs_ok  :zs_not_found);
#else
	struct stat64 buffer;   
	if (stat64(in_filepath, &buffer) == -1)
	{
		printf("errno=%d\n", errno);
		return z_get_os_error();
	}
	return zs_ok;

#endif

}

z_status __cdecl z_filesys_get_current_dir(char* dir,int length)
{
#ifdef BUILD_VSTUDIO
	if(_getcwd(dir,length)==NULL)
#else
	if(getcwd(dir,length)==NULL)
#endif
		return zs_bad_parameter; //TODO check for EACCES
	return zs_ok;
}



z_status  z_file_open_and_read(utf8 in_filepath, size_t * bytesread, char** pbuffer)
{
#ifdef BUILD_VSTUDIO
#ifdef _UNICODE
	SECURITY_ATTRIBUTES sa;
	int size;
	HANDLE handle;

	WCHAR* w_filepath=WCHAR_str_allocate(in_filepath,Z_MAX_PATH_LENGTH);

	handle= CreateFile(w_filepath, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL /*&sa*/, OPEN_EXISTING, 
		FILE_FLAG_SEQUENTIAL_SCAN, 0);

	WCHAR_str_deallocate(w_filepath);

	memset(&sa,0,sizeof(SECURITY_ATTRIBUTES));
	if(handle==INVALID_HANDLE_VALUE) return zs_could_not_open_file;
	size=GetFileSize(handle,NULL);
	*pbuffer =(char*)z_new char[size+1];
	BOOL result = (ReadFile(handle, *pbuffer, size, bytesread, NULL));
	(*pbuffer)[*bytesread] = 0;
	CloseHandle(handle);
	if(result)	
		return zs_ok;
	return zs_read_error;
#else
	SECURITY_ATTRIBUTES sa;
	int size;
	HANDLE handle;


	handle = CreateFile(in_filepath, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL /*&sa*/, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, 0);


	memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
	if (handle == INVALID_HANDLE_VALUE) return zs_could_not_open_file;
	size = GetFileSize(handle, NULL);
	*pbuffer = (char*)z_new char[size + 1];
	DWORD len;
	BOOL result = (ReadFile(handle, *pbuffer, size, &len, NULL));
	(*pbuffer)[len] = 0;
	CloseHandle(handle);
	if (result)
	{
		*bytesread = len;
		return zs_ok;
	}
		
	return zs_read_error;

#endif
#else
	FILE* pFile=fopen(in_filepath,"r");
	if (pFile == NULL)
		return zs_could_not_open_file;
	size_t len;

	fseek(pFile,0,SEEK_END);
	len=ftell(pFile);
	//printf("len=%d\n",len);
	*pbuffer =(char*)new char[len+1];
	//printf("*pbuffer=%p\n", *pbuffer);
	rewind(pFile);
	*bytesread=fread(*pbuffer,1,len,pFile);
	//printf("*bytesread=%ld\n", *bytesread);
	(*pbuffer)[len]=0;	
	//printf("bytesread=%lu\n",*bytesread);
	fclose(pFile);

	return zs_ok;
#endif

}

/*
void OutFile(ctext out_filepath,outmemory& out)
{
outfile of(out_filepath);
try 
{
of.open();
//int status=of.get_status();
of.put(out.get_data());
//zout.put(out.get_data());
}
catch (estream* e) 
{
zout.putf("Error: %s\n", (ctext)e->get_message());
delete e;
}
}
*/



z_status z_file_delete(utf8 name)
{
	z_status status=zs_unknown_error;
#ifdef BUILD_VSTUDIO
	ULONG error;
#ifdef _UNICODE


	WCHAR* w_filepath=WCHAR_str_allocate(name,Z_MAX_PATH_LENGTH);
	ZT("%s\n",name);
	if(DeleteFile(w_filepath)==TRUE) 
		status= zs_ok;
	else
	{
		error=GetLastError();
		if(error==ERROR_FILE_NOT_FOUND) 
			status= zs_not_found;

	}
	WCHAR_str_deallocate(w_filepath);
#else
	ZT("%s\n", name);
	if (DeleteFile(name) == TRUE)
		status = zs_ok;
	else
	{
		error = GetLastError();
		if (error == ERROR_FILE_NOT_FOUND)
			status = zs_not_found;

}
#endif
#else
	if(remove(name)==0)
		status= zs_ok;

#endif
	if(status)
		return Z_ERROR(status);
	return status;
}

z_status z_directory_delete_tree(utf8 path)
{
	z_directory_h dir;
	int type;
	ctext pname;
	z_status status;
	ZT(" %s\n",path);
	status=z_dir_open(path,&dir);
	if(status)
		return status;

	while(z_dir_get_next(dir,&pname,Z_DIR_TYPE_FILE|Z_DIR_TYPE_DIR,&type)==zs_ok)
	{
		z_string subpath=path;
		subpath<<'/'<<pname;
		if(type== Z_DIR_TYPE_DIR)
		{
			status=z_directory_delete_tree(subpath);
		}
		else
		{
			status=z_file_delete(subpath);
		}
		if(status)
			return status;
	}
	z_dir_close(dir);
	return z_directory_delete(path);;

}

z_status z_directory_delete(utf8 name)
{
	z_status status=zs_unknown_error;
	ZT(" %s\n",name);
#ifdef BUILD_VSTUDIO
#ifdef _UNICODE
	WCHAR* w_filepath=WCHAR_str_allocate(name,Z_MAX_PATH_LENGTH);
	if(RemoveDirectory(w_filepath)==TRUE) 
		status= zs_ok;
	else
	{
		/*
		z_string msg;
		status=z_get_os_error(&msg);
		Z_ERROR_MSG(status,"%s",msg.c_str());
		*/
	}
	WCHAR_str_deallocate(w_filepath);
#else
	if (RemoveDirectory(name) == TRUE)
		status = zs_ok;
#endif

#else
	//TODO! Handle linux errors
	if(remove(name)==0)
		status= zs_ok;

#endif
	return status;
}


z_status    z_dir_create(utf8 dir_name, int ignore_if_exists)
{
	int result = 0;
#if defined  BUILD_VSTUDIO ||  defined( BUILD_MINGW) || defined(BUILD_VX)
	result = _mkdir(dir_name);

#else
	result=mkdir(dir_name, 0xffff /*S_IRUSR| S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH*/);

#endif

	if (result)
	{
		switch (errno)
		{
		case EEXIST:
			if (ignore_if_exists)
				return zs_ok;
			return zs_already_exists;
		case ENOENT:
			return zs_could_not_open_dir;
		default:
			return zs_could_not_create_dir;
		}
	}
	return zs_ok;
}
z_status    z_directory_change(utf8 dir_name,int create)
{
	if(_chdir(dir_name)==0) 
		return zs_ok;
	if(!create) 
		return zs_could_not_open_dir;
	if(z_dir_create(dir_name,0))
		return zs_could_not_create_dir;
	if(_chdir(dir_name))
		return zs_could_not_open_dir;
	return zs_ok;
}
typedef struct _z_directory_t
{
	char* path;
#ifdef BUILD_VSTUDIO
	WIN32_FIND_DATA FindFileData;
	HANDLE handleDirectory ;
#ifdef _UNICODE
	WCHAR* wc_path;
#endif

#else
	DIR* handleDirectory ;
	struct dirent* entry;
#endif


} _z_directory;

#ifdef BUILD_VSTUDIO
#ifdef _UNICODE
z_status z_dir_open(utf8 name,z_directory_h* h)
{
	U32 temp;
	int result=-1;
	WCHAR* wc_path;
	_z_directory* zdir;
	//DBG_OUT(("z_dir_open %s\n",name));
	wc_path= WCHAR_str_allocate( name,MAX_PATH);
	temp=GetFileAttributes(wc_path);
	if((temp==INVALID_FILE_ATTRIBUTES) || ((FILE_ATTRIBUTE_DIRECTORY&temp)==0))
	{
		WCHAR_str_deallocate(wc_path);
		return zs_could_not_open_dir;
	}

	zdir=(_z_directory*)z_new _z_directory;
	_snwprintf_s( wc_path, MAX_PATH/2, MAX_PATH, L"%S//*", name);
	zdir->handleDirectory=0;
	zdir->wc_path=wc_path;
	zdir->path=(char*)z_new char[MAX_PATH];

	*h=(z_directory_h)zdir;
	return zs_ok;
}
#else 
z_status z_dir_open(utf8 name, z_directory_h* h)
{
	U32 temp;
	int result = -1;
	_z_directory* zdir;
	//DBG_OUT(("z_dir_open %s\n",name));
	temp = GetFileAttributes(name);
	if ((temp == INVALID_FILE_ATTRIBUTES) || ((FILE_ATTRIBUTE_DIRECTORY&temp) == 0))
	{
		return zs_could_not_open_dir;
	}

	zdir = (_z_directory*)z_new _z_directory;
	zdir->handleDirectory = 0;
	zdir->path = (char*)z_new char[MAX_PATH];
	_snprintf_s(zdir->path, MAX_PATH - 1, MAX_PATH - 1, "%s//*", name);

	*h = (z_directory_h)zdir;
	return zs_ok;
}
#endif

#else 
z_status    z_dir_open(utf8 name,z_directory_h* h)
{
	_z_directory* zdir=(_z_directory*)malloc(sizeof(_z_directory));
	zdir->handleDirectory=0;
	ZT("z_dir_open %s\n",name);
	*h=(z_directory_h)zdir;
	zdir->handleDirectory=opendir(name);
	ZT("handleDirectory %x\n",zdir->handleDirectory);
	if (!zdir->handleDirectory)
		return zs_could_not_open_dir;
	return zs_ok;
}
#endif


#ifdef BUILD_VSTUDIO
#ifdef _UNICODE

z_status  z_dir_get_next(z_directory_h h,utf8* currentfile,int requestedtypes,int *typeout)
{
	int type;
	int handle_dir=0;
	_z_directory* zdir=(_z_directory*)h;
	//DBG_OUT(("z_dir_get_next\n"));
	while(1)
	{
		if(zdir->handleDirectory==0)
		{
			zdir->handleDirectory = FindFirstFile(zdir->wc_path, &(zdir->FindFileData));
			if(INVALID_HANDLE_VALUE==zdir->handleDirectory)
			{
				zdir->handleDirectory=0;
				return zs_end_of_list;
			}
		}
		else
			if (FindNextFile(zdir->handleDirectory, &(zdir->FindFileData))==0) 
			{
				return zs_end_of_list;
			}

			Unicode16ToUtf8(zdir->FindFileData.cFileName,zdir->path,MAX_PATH);
			*currentfile=zdir->path;
			type=((zdir->FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)?Z_DIR_TYPE_DIR:Z_DIR_TYPE_FILE);


			if(!(requestedtypes&type))
				continue;
			*typeout=type;
			if(strcmp(*currentfile,".")==0) continue;
			if(strcmp(*currentfile,"..")==0) continue;
			return zs_ok;
	}
	return zs_unknown_error; //TODO 
}
#else
z_status  z_dir_get_next(z_directory_h h, utf8* currentfile, int requestedtypes, int *typeout)
{
	int type;
	int handle_dir = 0;
	_z_directory* zdir = (_z_directory*)h;
	while (1)
	{
		if (zdir->handleDirectory == 0)
		{
			zdir->handleDirectory = FindFirstFile(zdir->path, &(zdir->FindFileData));
			if (INVALID_HANDLE_VALUE == zdir->handleDirectory)
			{
				zdir->handleDirectory = 0;
				return zs_end_of_list;
			}
		}
		else
			if (FindNextFile(zdir->handleDirectory, &(zdir->FindFileData)) == 0)
			{
				return zs_end_of_list;
			}

		type = ((zdir->FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? Z_DIR_TYPE_DIR : Z_DIR_TYPE_FILE);


		if (!(requestedtypes&type))
			continue;
		if (strcmp(zdir->FindFileData.cFileName, ".") == 0) continue;
		if (strcmp(zdir->FindFileData.cFileName, "..") == 0) continue;
		strncpy(zdir->path,zdir->FindFileData.cFileName,  MAX_PATH);
		*currentfile = zdir->path;

		*typeout = type;
		return zs_ok;
	}
	return zs_unknown_error; //TODO 
}
#endif

#else 



z_status  z_dir_get_next(z_directory_h h,utf8* currentfile,int requestedtypes,int *typeout)
{
	int type=0;;
	int handle_dir=0;
	_z_directory* zdir=(_z_directory*)h;
	ZT("z_dir_get_next\n");
	while(1)
	{
		struct stat status;
		zdir->entry=readdir(zdir->handleDirectory);
		ZT("zdir->entry=%x\n",zdir->entry);
		if(!zdir->entry)
			return zs_end_of_list;
		*currentfile=zdir->entry->d_name;
		ZT("currentfile=%x %s\n",currentfile,*currentfile);

		type=0;
#ifdef UNIX
		switch(zdir->entry->d_type)
		{
		case DT_DIR:
			type=Z_DIR_TYPE_DIR;
			break;
		case DT_REG:
			type=Z_DIR_TYPE_FILE;
			break;		
		default:
			type=0;
			break;
		}
		ZT("d_type=%x\n",zdir->entry->d_type);
#endif
		if(!type)
		{
			struct stat stat_struct;
			if(stat(*currentfile,&stat_struct)==0)
			{
				if(S_ISDIR(stat_struct.st_mode))
					type=Z_DIR_TYPE_DIR;
				if(S_ISREG(stat_struct.st_mode))
					type=Z_DIR_TYPE_FILE;
			}

		}
#if 0 //all else fails, try to open it as a dir
		handle_dir=opendir(zdir->entry->d_name);
		if(handle_dir) 
		{
			type= Z_DIR_TYPE_DIR;
			close(handle_dir);
		}
		else
			type= Z_DIR_TYPE_FILE;
#endif		

		ZT("type=%x\n",type);


		if(!(requestedtypes&type))
			continue;
		*typeout=type;
		if(strcmp(*currentfile,".")==0) continue;
		if(strcmp(*currentfile,"..")==0) continue;
		return zs_ok;
	}
	return zs_unknown_error; //TODO 
}
#endif

void   z_dir_close(z_directory_h h)
{
	_z_directory* zdir=(_z_directory*)h;
	if(!zdir) return;
#ifdef BUILD_VSTUDIO	
	if(zdir->path) 
	{
		free(zdir->path);
#ifdef _UNICODE	
		WCHAR_str_deallocate(zdir->wc_path);
#endif

		zdir->path=0;
	}
#endif

	if(zdir->handleDirectory) 
	{
#ifdef BUILD_VSTUDIO	
		FindClose(zdir->handleDirectory);
#else
		closedir(zdir->handleDirectory);
#endif
	}
	zdir->handleDirectory=0;

	free(zdir);
}

#if 0 //UNUSED
utf8 z_get_filename_from_path(utf8 fullpath)
{
#ifdef UNIX
	utf8 filename=strrchr(fullpath,'/');
#else
	utf8 filename=strrchr(fullpath,'\\');
#endif
	if(filename) filename++;
	else filename=fullpath;
	return filename;
}




int z_file_open_and_write(utf8 in_filepath,U8* data,unsigned long length  )
{
	U32 byteswritten=0;
#ifdef BUILD_VSTUDIO
	//SECURITY_ATTRIBUTES sa;

	WCHAR* w_filepath=WCHAR_str_allocate(in_filepath,Z_MAX_PATH_LENGTH);

	HANDLE handle= CreateFile(w_filepath, GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL /*&sa*/, CREATE_ALWAYS, 
		FILE_FLAG_SEQUENTIAL_SCAN, 0);	
	WCHAR_str_deallocate(w_filepath);

	//memset(&sa,0,sizeof(SECURITY_ATTRIBUTES));

	if(handle==INVALID_HANDLE_VALUE) return 0;
	if(WriteFile(handle,data,length,(LPDWORD)&byteswritten,NULL)) return byteswritten;
	return 0;

#else
	FILE* pFile=fopen(in_filepath,"wb");
	byteswritten=fwrite(data,1,length,pFile);

	fclose(pFile);

	return byteswritten;
#endif

}

z_status z_fopen(z_file_h* filep,utf8 _Filename,ascii _Mode)
{
#ifdef BUILD_VSTUDIO
	return fopen_s((FILE**)filep,_Filename,_Mode);
#else
	*filep=(z_file_h)fopen(_Filename,_Mode);
	if(*filep == 0) return -1;
	return 0;
#endif
}

#endif