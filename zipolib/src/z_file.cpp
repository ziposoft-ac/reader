#include "pch.h"
#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include "zipolib/z_file.h"

z_status z_FileReadAllLines(ctext filename, z_strlist &list)
{
	z_string line;

	std::ifstream file(filename);
	if (!file.is_open())
	{
		return zs_could_not_open_file;
	}
	while (std::getline(file, line))
	{

		list << line;
	}
	return zs_ok;

	



}
z_status z_file_save_from_stdin(ctext filename, uint len)
{
	z_status status = zs_ok;;
	std::ofstream file(filename);
	uint buffsize = 0x100000;
	if (len < buffsize)
		buffsize = len;
#ifdef WIN32
	_setmode(_fileno(stdin), _O_BINARY);
#endif
	char* buff = z_new char[buffsize + 1];
	while (len>0)
	{
		uint readlen = 0x10000;
		if (len < readlen)
			readlen = len;

		uint bytesread = fread(buff, 1, readlen, stdin);
		if (bytesread != readlen)
		{
			Z_ERROR_MSG(zs_read_error, "Read %x bytes, expected %x", bytesread, readlen);
			status = zs_read_error;
			break;

		}
		file.write(buff, bytesread);

		len = len - readlen;
	}
	z_delete buff;
	return status;

}

#if OLD
using namespace std;
#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
/*________________________________________________________________________

								Globals

________________________________________________________________________*/

z_file& z_stdin_get()
{
	static z_file z_static_in((size_t)stdin);
	return z_static_in;


}
z_file& z_stdout_get()
{
	static z_file z_static_out((size_t)stdout);
	return z_static_out;

}

/*________________________________________________________________________

								z_file	 Init

________________________________________________________________________*/

z_file::z_file()
{
	init();
}
z_file::z_file(ctext filename)
{
	init();
	_file_name=filename;
}
z_file::~z_file()
{
	close();
}
z_file::z_file(ctext filename,ctext mode)
{
	init();
	_file_name=filename;
	_file_handle=(size_t)(void*)fopen(filename,mode);
}
z_file::z_file(size_t h)
{
	init();
	_file_handle=h;
}
void z_file::init()
{
	_file_handle=0;
	_log_file_handle=0;
	_max_line_length=0x1000;
	_indent_depth=0;
   _buffer = 0;
}
void z_file::set_handle(size_t h)
{
	_file_handle=h;
}

/*________________________________________________________________________

								z_file	 Info

________________________________________________________________________*/

bool z_file::get_file_size(size_t &size)//TODO more efficient
{
	if (_file_handle==0) return false;
	::fseek((FILE*)_file_handle,0L,SEEK_END);
	size=::ftell((FILE*)_file_handle);
	::rewind((FILE*)_file_handle);
	return true;
}



/*________________________________________________________________________

								z_file	 Open/Close

________________________________________________________________________*/


z_status z_file::open(ctext filename,ctext mode)
{
	_file_name=filename;
	_file_handle=(size_t)(void*)fopen(filename,mode);
	if( _file_handle==0) return zs_could_not_open_file;
	return zs_ok;
}

void z_file::close()
{
	if(_file_name) //dont close stdin/stdout
		if(_file_handle)
		{
			fclose((FILE*)_file_handle);
			_file_handle=0;

		}
}


/*________________________________________________________________________

								z_file	 Read

________________________________________________________________________*/

bool z_file::read_all(z_string & str)
{
	size_t size;
	if(!get_file_size(size)) 
		return false;
	char* buff=z_new char [size+1];
	read(buff,size);
	buff[size]=0;
	str=buff;
	delete buff; // TODO - MUST be a better way to do this

	return true;
}
bool z_file::read_all(char* & data,size_t& size)
{
	if(!get_file_size(size)) 
		return false;
	data=(char*)z_new char [size+1];
	read(data,size);
	data[size]=0;
	return true;
}

bool z_file::rewind()
{
	::rewind((FILE*)_file_handle);
	return true;
}

size_t z_file::getline(char* buff,size_t size)
{
	size_t i;
	if (0==buff) return 0;
	if(fgets( buff, (int)size, (FILE*)_file_handle )==0)
		return 0;
	i=strlen(buff);
	while( (i>0)&&
		   (
		     (buff[i-1]=='\n')||
		     (buff[i-1]=='\r')
		   )
		 )
	{
		buff[i-1]=0;
		i--;
	}
	return i; //TODO performance- better way to know how much we read?
}
size_t z_file::read(char* buff,size_t size)
{
	return fread( buff, 1,size, (FILE*)_file_handle );
}


char z_file::get(char& c)
{
	c=fgetc((FILE*)_file_handle);
	return c;
}

z_status z_file::getline(z_string& s)
{
	char* tb=z_temp_buffer_get(_max_line_length);
	char* val=0;
	if(!_file_handle)
		return zs_not_open;
	val=fgets( tb, _max_line_length, (FILE*)_file_handle );
	if(!val)
	{
		if(feof( (FILE*)_file_handle))
			return 	zs_eof;
		//TODO handle errors
		return zs_read_error;
	}
	size_t l=strlen(tb);
	if(tb[l-1]=='\n')
		tb[l-1]=0;

	s=tb;
	z_temp_buffer_release(tb);
	return zs_ok;
}






/*________________________________________________________________________

		z_file - Writing
________________________________________________________________________*/
void z_file::flush()
{
    
#ifdef BUILD_VSTUDIO
    FlushFileBuffers(HANDLE(_file_handle));
#else
	Z_ASSERT(0);
#endif
}

int z_file::write(const char* buf, size_t count )
{
	if(!_file_handle)
	{
		return -1;
	}
	::fwrite(buf,1,count,(FILE*)_file_handle);	
	if(_log_file_handle)
	{
		::fwrite(buf,1,count,(FILE*)_log_file_handle);	

	}
	return 0;
}

int z_file::putf(const char*  lpszFormat,  ...  )
{
    int c;
    va_list ap;
	char* tempbuf=(char*)z_temp_buffer_get(_max_line_length);
    va_start (ap, lpszFormat);
    c=vsnprintf (tempbuf,_max_line_length, lpszFormat, ap);
    va_end (ap);
    write(tempbuf,strlen(tempbuf));
	z_temp_buffer_release(tempbuf);
    return c;
}
void z_file::eol()
{
#ifdef BUILD_VSTUDIO
	write("\r\n",2);
#else
	write("\n",1);
#endif
}
/*________________________________________________________________________

		z_file - Writing  String Output
________________________________________________________________________*/

void z_file::indent()
{
	int i=_indent_depth;
	while(i--)
		*this<<"  ";
}
void z_file::indent_inc()
{
	_indent_depth++;

}
void z_file::indent_reset()
{
	_indent_depth=0;
}
void z_file::indent_dec()
{
	_indent_depth--;

}

/*________________________________________________________________________

		z_file - Misc
________________________________________________________________________*/
z_status z_file::delete_file()
{
	close();
	return z_file_delete(_file_name);
}
void z_file::start_log_to_file(ctext filename)
{
#if 0
	_log_file_handle=(void*)(size_t)_open(filename,_O_CREAT | _O_TRUNC|_O_BINARY );
#else
	_log_file_handle=(void*)fopen(filename,"ab");
#endif
}
void z_file::stop_log_to_file()
{
	if(_log_file_handle)
	fclose((FILE*)_log_file_handle);
	_log_file_handle=0;
}


z_status z_file::mmap_open(U8** map_ptr_out,bool readonly)
{

return Z_ERROR_NOT_IMPLEMENTED;
}
z_status z_file::mmap_close(U8* map_ptr)
{
return Z_ERROR_NOT_IMPLEMENTED;

}
#endif