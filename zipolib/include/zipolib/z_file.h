/*________________________________________________________________________

z_file.h

________________________________________________________________________*/


#ifndef z_file_header
#define z_file_header
#include "zipolib/zipo.h"

#include "zipolib/z_strlist.h"
#include "zipolib/z_error.h"

class z_file
{
protected:
public:




};
z_status z_FileReadAllLines(ctext file, z_strlist &list);

class z_file_text_in
{
protected:
public:
	z_status readIn(ctext name);




};

z_status z_file_save_from_stdin(ctext filename, uint len);
#if OLD

/*________________________________________________________________________

								z_file
________________________________________________________________________*/
class z_file
{
	size_t  _file_handle;	
	z_string _file_name;
	void* _log_file_handle;
	int _indent_depth;
   char* _buffer;
   char* _get_buffer();
protected:
	int _max_line_length;
public:
	static const int 
		flag_write=0x001,
		flag_read=0x002,
		flag_append=0x004,
		flag_share_read=0x008, 
		flag_overwrite=0x010,	
		flag_create_new=0x020,	
		flag_open_always=0x040,	
		flag_delete_on_close=0x080,	
		flag_temporary=0x10000;	
	/*______________________________________

				Init
	  ______________________________________*/	
    z_file();
    z_file(size_t file_handle);
    z_file(ctext filename);
    z_file(ctext filename,ctext mode);
    virtual ~z_file();

	void init();

	void set_handle(size_t t);
	/*______________________________________

				Info
	  ______________________________________*/	
	bool get_file_size(size_t &size);
	bool is_open() { return _file_handle!=0; }



	/*______________________________________

				Open/Close
	  ______________________________________*/	
	z_status open(ctext filename,ctext mode);
	void close();
	/*______________________________________

				Reading
	  ______________________________________*/	
	bool rewind();
	char get(char& c);
	size_t getline(char* buff,size_t size);
	z_status  getline(z_string & str);
	size_t read(char* buff,size_t size);
	bool read_all(z_string & str);
	bool read_all(char*& data,size_t& size);


	/*______________________________________

				Writing
	  ______________________________________*/	
	virtual int  write(const char* buf, size_t count);
    virtual int putf(const char*  lpszFormat,  ...  );
	void flush();

 	/*______________________________________

				String Output
	  ______________________________________*/	

	//TODO - This is awful! Fix this
	template <class TYPE>  z_file  &put(TYPE data)
	{ 
		char* tb=(char*)z_temp_buffer_get(0x100);
		z_convert(data,tb,0x100);
		write(tb,strlen(tb));
		z_temp_buffer_release(tb);
		return *this;
	}		
	
	virtual z_file  &operator <<  (double x) { return put(x); }
    virtual z_file  &operator <<  (const char x)
	{ 
    	write(&x,1);
		return *this;
	}
    virtual z_file  &operator <<  (const char* x)
	{ 
    	write(x,strlen(x));
		return *this;
	}
    virtual z_file  &operator <<  (const z_string &x)
	{ 
    	write(x.c_str(),x.length());
		return *this;
	}
    virtual z_file  &operator <<  (I64 x){ return put(x); }
    virtual z_file  &operator <<  (I32 x){ return put(x); }
    virtual z_file  &operator <<  (U64 x){ return put(x); }
    virtual z_file  &operator <<  (U32 x){ return put(x); }
	void indent();
	void indent_inc();
	void indent_reset();
	void indent_dec();	
	void eol();
	
	/*______________________________________

				Misc
	  ______________________________________*/	
	z_status delete_file();

	z_status mmap_open(U8** map_ptr_out,bool readonly);
	z_status mmap_close(U8* map_ptr);

	virtual void start_log_to_file(ctext filename);
	virtual void stop_log_to_file();


};

#endif
#endif

