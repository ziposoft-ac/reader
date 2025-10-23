#ifndef z_parse_xml_h
#define z_parse_xml_h
#include "zipolib/parse_objs.h"


class XmlBase
{
public:

	XmlBase()
	{

	}
	virtual ~XmlBase()
	{
	}

};

class XmlElm
{
public:
	z_string _name;

	XmlElm()
	{
		_child = 0;

	}
	virtual ~XmlElm()
	{
		if (_child)
			delete _child;
	}

	XmlElm* _child;
};
ZMETA(XmlElm)
{
	ZPROP(_name);
	ZPROP(_child);
};


#endif

