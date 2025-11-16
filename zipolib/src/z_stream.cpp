#include "pch.h"
#include "zipolib/z_error.h"
#include "zipolib/z_strlist.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/poll.h>
#include <sys/eventfd.h>

//#include <memory>
//#include <stdarg.h>
using namespace std;


ctext z_get_filename_from_path(ctext fullpath)
{
#ifdef UNIX
	ctext filename = strrchr(fullpath, '/');
#else
	ctext filename = strrchr(fullpath, '\\');
#endif
	if (filename) filename++;
	else filename = fullpath;
	return filename;
}
/*________________________________________________________________________

z_stream
________________________________________________________________________*/


void z_stream::write_std(const std::string& x)
{
	write(x.c_str(), x.size());
}
z_status z_stream::write_safe(const char* data, size_t len)
{
	return write(data,len);
}
z_stream & z_stream::operator << (const z_string & x)
{
	write_std(x);
	return *this;
}
z_stream & z_stream::operator << (const std::string & x)
{
	write_std(x);
	return *this;
}

z_stream & z_stream::operator << (char x)
{
	write_safe(&x,1);
	return *this;

}
z_stream & z_stream::operator << (const char* x)
{
	write_str(x);

	return *this;
}
z_stream & z_stream::operator << (int x)
{
	write_std(to_string(x));
	return *this;
}

z_stream & z_stream::operator << (float x)
{
	write_std(to_string(x));
	return *this;
}
z_stream& z_stream::operator << (long double x)
{
	write_std(to_string(x));
	return *this;
}
z_stream & z_stream::operator << (double x)
{
	write_std(to_string(x));
	return *this;
}
z_stream & z_stream::operator << (U16 x)
{
	write_std(to_string(x));
	return *this;
}

z_stream & z_stream::operator << (int64_t x)
{
	write_std(to_string(x));
	return *this;
}
z_stream & z_stream::operator << (uint32_t x)
{
	write_std(to_string(x));
	return *this;
}
z_stream & z_stream::operator << (uint64_t x)
{
	write_std(to_string(x));
	return *this;
}

z_stream::z_stream()
{

	
}
z_stream::~z_stream()
{
}


#define BUFF_SIZE 200

bool z_stream::format_args(ctext pFormat, va_list ArgList)
{
	static uint buff_size = BUFF_SIZE;
	int length_written;
	int i = 2;
	while (i--)
	{
		length_written = try_format_args(buff_size, pFormat, ArgList)+1; // Extra space for '\0'
		if (length_written < buff_size)
			break;
		buff_size = length_written;
	}
	return true;

}
/*
returns zero on success or new required buff size
*/
int z_stream::try_format_args(int buff_size, ctext pFormat, va_list ArgList)
{
	int result = 0;
	int length_to_write;
	char* buff= z_new char[buff_size+1];

	length_to_write = vsnprintf(buff, buff_size, pFormat, ArgList); // Extra space for '\0'
	if (length_to_write < 0)
	{
		//error  THROW
		return 0;
	}
	if ((length_to_write) > buff_size)
	{
		result = length_to_write;
	}
	else
		write_str(buff, length_to_write);
	delete []buff;

	return result;


}
void z_stream::indent(int tabs)
{
	if ((tabs > 100)||(tabs<0))
	{
		// TODO!
		//throw std::exception("bad tabs");
		return;
	}
	while (tabs--)
		write_safe("  ", 2);
}
z_stream & z_stream::format_append(ctext pFormat, ...)
{
	static uint buff_size = BUFF_SIZE;
	va_list ArgList;
	int i = 2;
	int result;
	while (i--)
	{
		va_start(ArgList, pFormat);
		result = try_format_args(buff_size,pFormat, ArgList);
		va_end(ArgList);
		if (result==0)
			break;
		buff_size = result+2;

	}
	return *this;
}
z_stream & z_stream::dump_hex(U8* data,size_t len) {
	for (int i=0;i<len;i++) {
		U8 b=data[i];
		format_append("%02x ",b);


	}
	return *this;

}



void  z_stream::trace(ctext file, ctext func, int line, bool endline)
{
	*this << z_get_filename_from_path(file) << '(' << line << ") " << func << "()";
	if (endline)
		*this << "\n";

}
void z_stream::trace_v(ctext file, ctext func, int line, ctext pFormat, ...)
{
	static int buff_size = BUFF_SIZE;
	*this << z_get_filename_from_path(file) << '(' << line << ") " << func << "()";
	va_list ArgList;
	int result;
	int i = 2;
	while (i--)
	{
		va_start(ArgList, pFormat);
		result = try_format_args(buff_size,pFormat, ArgList);
		va_end(ArgList);
		if (result == 0)
			break;
		buff_size = result;
	}
	*this << "\n";

}

static  uint g_buff_size = BUFF_SIZE;


z_status z_stream::write_str(const char* data, size_t len)
{
	if (len == -1)
		len = strlen(data);
	if(len)
		write_safe(data, len);
	return zs_ok;

}

#ifdef WINDOWS
/*________________________________________________________________________

z_stream_windbg
________________________________________________________________________*/

z_status z_stream_windbg::write(const char* data, size_t len)
{
	std::unique_lock<std::mutex> mlock(_mutex);

	char* buff = z_new char[len + 1];
	memcpy(buff, data, len);
	buff[len] = 0;

	OutputDebugStringA(buff);
	delete buff;
	return zs_ok;

}

z_status z_stream_windbg::write_str(const char* data, size_t len )
{
	std::unique_lock<std::mutex> mlock(_mutex);
	OutputDebugStringA(data);
	fwrite(data, len, 1, stderr);

	return zs_ok;
}
#endif


/*________________________________________________________________________

z_stream_multi
________________________________________________________________________*/

z_status z_stream_multi::write(const char* data, size_t len)
{
	for (auto i : _streams)
	{
		i->write(data, len);
		
	}
	return zs_ok;
}
void z_stream_multi::flush() {
	for (auto i : _streams)
	{
		i->flush();

	}
}

/*________________________________________________________________________

z_stream_stdout
________________________________________________________________________*/


void z_stream_stdout::flush()
{
	fflush(stdout);
}

z_status z_stream_stdout::write(const char* data, size_t len)
{
	fwrite(data, len, 1, stdout);
	//cout.write(data, len);
	return zs_ok;
}
z_stream_stdout gz_stdout;
/*________________________________________________________________________

z_stream_stdout
________________________________________________________________________*/


void z_stream_stderr::flush()
{
	fflush(stderr);
}

z_status z_stream_stderr::write(const char* data, size_t len)
{
	fwrite(data, len, 1, stderr);
	//cout.write(data, len);
	return zs_ok;
}
z_stream_stderr gz_stderr;


z_stream_null gz_stream_null;
/*________________________________________________________________________

z_stream_error
________________________________________________________________________*/
z_stream_error::z_stream_error()
{
	//_streams.add(&gz_stderr);

}

z_stream_error gz_stream_error;
/*________________________________________________________________________

z_file_out
________________________________________________________________________*/
z_file_out::z_file_out(ctext filename)
{
	// TODO need to throw error 
	open(filename);

}
z_file_out::z_file_out() 
{

	_file = 0;

}
z_file_out::~z_file_out()
{

	close();

}

void z_file_out::close()
{
	if (_file)
		fclose(_file);
	_file = 0;
}
bool z_file_out::is_open()
{
	return _file != 0;
}
z_status z_file_out::open(ctext filename, ctext mode)
{
#ifdef WINDOWS
	_file = _fsopen(filename, mode, _SH_DENYNO);
	if (!_file)
	{
		return Z_ERROR_MSG(zs_could_not_open_file, "err=%d", errno);
	}
#else
	_file = fopen(filename, mode);
	if(!_file)
    {
	    switch(errno)
        {
            case EACCES:
                return zs_access_denied;
                break;
            case ENOENT:
                return zs_not_found;
            default:
                return zs_could_not_open_file;
        }



    }

#endif
	return zs_ok;

}

z_status z_file_out::write(const char* data, size_t len)
{
	if (!_file)
		return zs_not_open;
	uint len_written=fwrite(data, len, 1, _file);
	if (len_written != len)
		return zs_write_error;

	return zs_ok;
}
z_status z_file_out::write_str(const char* data, size_t len)
{
	if (len == -1)
		len = strlen(data);
	return write(data, len);

}
void z_file_out::flush()
{
    fflush(_file);
}

void z_stream_debug::flush()
{
	if (_fPipe>0)
		::fsync(_fPipe);
}
z_status z_stream_debug::open()
{
    if(_fPipe==0)
    {
        mkfifo("/tmp/debugview",0666);
        _fPipe = ::open("/tmp/debugview",(  O_WRONLY | O_NONBLOCK));
    	if (_fPipe<0) {
    		if (errno==6) {
    			printf("debugview app is not open\n");
    			return zs_ok;
    		}
    		printf("could not open debugview pipe err=%d %s\n", errno,strerror(errno));
    		return Z_ERROR_MSG(zs_could_not_open_file, "could not open debugview pipe err=%d", errno);

    	}
    }
    return zs_ok;
}
z_status z_stream_debug::write(const char* data, size_t len)
{
	if (_fPipe<0)
		return zs_could_not_open_file;
    if (_fPipe==0)
        open();
	if (_fPipe>0)
		::write(_fPipe, data,len);



    return zs_ok;
}