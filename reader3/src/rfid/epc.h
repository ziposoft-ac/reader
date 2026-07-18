#ifndef epc_h
#define epc_h
#include "pch.h"

/*______________________________________________________________
30 34 36 49 70 3A CE C8 00 00 00 DC
 0 1  2  3  4  5  6  7  8  9  10 12 
                  Epc
________________________________________________________________*/
class  Epc
{
public:
    static const int _max_len=32;
private:
	U8 _len=0;
	U8 _buff[_max_len+1];
	U16 _bib_number = 0;
	U16 get_int_from_bcd();

public:

    Epc() {}
	Epc(ctext epx_hex)
	{
        setFromHexString(epx_hex);
	}
	Epc(const Epc &other)
	{
		copy(other);
	}
	Epc(const U8* buff, size_t len)
	{
		set_bytes(buff, len);
	}
	~Epc()
	{


	}
	void invalidate();
	friend z_stream& operator<<(z_stream& out,  Epc &epc);

	friend bool operator== (const Epc &c1, const Epc &c2);
	Epc& operator= (const Epc &epc);
	Epc& operator= (ctext epx_hex);
	Epc& copy(const Epc &epc);
	Epc& move(Epc &epc);
	U16 get_bib_number();
	bool is_valid();
	Epc& set_bytes(const U8* buff, size_t len);
	Epc& setFromHexString(ctext hex_string);

	void set_bcd_from_int(U16 number);
	//void set_cgtc(U16 number);


    void getHexString(z_string &s);
    z_string& getAsciiString(z_string &s);

	size_t get_len() { return _len; }
	U8* get_data() { return _buff;  }

};

#endif
