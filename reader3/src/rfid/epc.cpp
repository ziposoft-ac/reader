#include "pch.h"
#include "epc.h"

#include <c++/14/cassert>


//ctext cgtc_header = "CGTC";
//const U32 cgtc_header_word = *(U32*)cgtc_header;
//const U32 cgtc_header_word = 0xcefaedfe;
							


/*______________________________________________________________________________________________


				Epc

______________________________________________________________________________________________*/
void Epc::set_bcd_from_int(U16 number)
{
    _len = 4;// many chips have 4 byte minimum
    //_len = 2;
	char a1 = number / 1000;
	char a2 = (number % 1000) / 100;
	char a3 = (number % 100) / 10;
	char a4 = (number % 10);

	_buff[0] = 0;
	_buff[1] = 0;
	_buff[2] = a1 * 16 + a2;
	_buff[3] = a3 * 16 + a4;
     /*
    _buff[0] = a1 * 16 + a2;
    _buff[1] = a3 * 16 + a4;
      */
}

Epc& Epc::copy(const Epc &other)
{
	invalidate();
	_len = other._len;
	memcpy(_buff, other._buff, _len);
	return *this;
}
Epc& Epc::set_bytes(const U8* buff, size_t len)
{

	invalidate();
	if(len>_max_len)
    {
	    Z_WARN_MSG(zs_bad_parameter,"%d exceeds max EPC length %d",len,_max_len);
        len=_max_len;
    }
    if(buff)
    {
        memcpy(_buff, buff, len);
        _len = len;
    	_buff[len]=0;
    }

	return *this;
}




bool operator== (const Epc &c1, const Epc &c2)
{
	size_t len = c1._len;
	if (len != c2._len)
		return false;
	return (memcmp(c1._buff, c2._buff, len) == 0);
}
Epc& Epc::operator= (const Epc &other)
{
	return copy(other);
}
Epc& Epc::operator= (ctext epx_hex)
{
    setFromHexString(epx_hex);
	return *this;

}
z_stream& operator<<(z_stream& out,  Epc &epc)
{


    z_string s;
    //out << epc.getAsciiString(s);

    epc.getHexString(s);
	out << s;


	return out;
}


void Epc::invalidate()
{

	_len = 0;
}
#if 0
void Epc::set_cgtc(U16 number)
{
	invalidate();
	_len = 12;
	_buff = z_new U8[_len];
	memset(_buff, 0, _len);
	*(U32*)_buff = cgtc_header_word;
	*(U16*)(_buff + 6) = number;


}
#endif


U16 Epc::get_int_from_bcd()
{
	int i;
	int deci = 1;
	int val = 0;
	for (i = (_len-1); i >= 0; --i)
	{
		
		int digit0 = _buff[i] & 0xf;
		int digit1 = (_buff[i] & 0xf0)>>4;
		

		// Invalid BCD
		if (digit0 > 9) return 0;
		if (digit1 > 9) return 0;
		
		val = val + (digit0 + digit1 * 10)*deci;

		deci = deci * 100;
	}
	return val;

}
U16 Epc::get_bib_number()
{
	if (!_bib_number)
	{
		/*
		
		valid bib numbers are 2 or 4 byte BCD 
		0024
		00009873

		*/
		if ((_len != 8) &&(_len != 4) && (_len != 2))
		{
			return 0;

		}

		_bib_number = get_int_from_bcd();
	}

	return _bib_number;
	/*
	
	if (!is_valid())
		return 0;
	U16  number = *(U16*)(_buff + 6);
	return number;	
	*/

}
bool Epc::is_valid()
{
	
	return get_bib_number() > 0;
//	U32 header_word = *(U32*)_buff;
//	return (header_word == cgtc_header_word);
}
void Epc::getHexString(z_string &s)
{
	if(_len>_max_len)
	{
		Z_WARN_MSG(zs_bad_parameter,"ERROR! %d exceeds max EPC length %d",_len,_max_len);
		_len=_max_len;
	}
    s.resize(_len*2);
    char* strbuff=s.data();
    constexpr char hexval[]="0123456789ABCDEF";
    for(int j = 0; j < _len; j++){
        strbuff[j*2] = hexval[((_buff[j] >> 4) & 0xF)];
        strbuff[(j*2) + 1] = hexval[(_buff[j]) & 0x0F];
    }
	strbuff[_len*2]=0;


}
z_string& Epc::getAsciiString(z_string &s)
{
    s.resize(_len);
    U8* strbuff=(U8*)s.c_str();
    for(int j = 0; j < _len; j++){
        U8 byte= _buff[j];
        if((byte<32)||(byte>126))
        {
            byte='*';
        }
        strbuff[j] = byte;
    }
    return s;

}


int char2int(char input)
{
    if(input >= '0' && input <= '9')
        return input - '0';
    if(input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if(input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    throw std::invalid_argument("Invalid input string");
}
Epc& Epc::setFromHexString(ctext epc_hex)
{
    invalidate();
    size_t len_hex = strlen(epc_hex);
    if (len_hex % 4)
    {
        Z_ERROR_MSG(zs_bad_parameter, "Invalid length EPC hex string");
    }
    int i=0;
    ctext a=epc_hex;
    while(*a && a[1])
    {
        _buff[i] = char2int(*a)*16 + char2int(a[1]);
        a += 2;
        if(++i>= _max_len)
        {
            Z_WARN_MSG(zs_bad_parameter, "EPC hex string exceeds length");

            break;

        }
    }
    _len=i;

    return *this;
}
