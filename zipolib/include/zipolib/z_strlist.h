/*________________________________________________________________________

 z_list_h

________________________________________________________________________*/



#ifndef z_strlist_h
#define z_strlist_h
/*_______________________________________________________________________*\



\*_______________________________________________________________________*/
#include "zipolib/zipo.h"
#include "zipolib/z_string.h"

class z_file;

class z_strlist : public std::vector<z_string>
{
public:
    z_strlist()
    {

    }
    virtual ~z_strlist()
    {

    }
	 
    int del(uint i)
    {
        erase(begin()+i);
        return 0;
    }
    int find(const char* str)
    {
		//TODO ugly
		return find(z_string(str));

    }
    int find(const z_string &str)
    {
		iterator i;
		i = std::find(begin(), end(), str);
		if (i == end()) return -1;
		return (int)(i - begin());
	}
	int break_string(ctext s,char c);
	void get_as_string(z_string &_out) const;
	operator bool() const 
	{ 
		return (size()>0);
	}
	virtual z_strlist & operator << (z_string s) { push_back(s); return *this; }
	virtual z_strlist & operator << (ctext s) { push_back(s); return *this; }
	z_strlist & operator = (ctext s);


};


#endif

