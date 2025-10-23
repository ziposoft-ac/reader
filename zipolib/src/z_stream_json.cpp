#include "pch.h"
#include "zipolib/z_stream_json.h"
#include "zipolib/z_strlist.h"
z_json_stream stdout_json(zout);

z_string z_json_stream::escape(ctext input)
{
	z_string s;

	size_t i;
	for (i = 0; i<strlen(input); i++)
	{
		char c = input[i];
		switch (c)
		{
		case '\\':
			s.append("\\\\");
			break;
		case '\n':
			s.append("\\n");
			break;
		case '\r':
			s.append("\\r");
			break;
		case '\t':
			s.append("\\t");
			break;

		case '\"':
			s.append("\\\"");
			break;
		default:
			s.append(1, c);
			break;
		}
	}
	return s;








}



z_json_stream::z_json_stream(z_stream& s,bool pretty_print) : _out(s)
{

	_depth = 0;
	_need_comma = false;
	_pretty_print = pretty_print;
	_pretty_print_array = false;
}
z_json_stream:: ~z_json_stream()
{

}
// returns previous value, for save/restore
bool z_json_stream::set_pretty_print(bool on)
{
	bool prev = _pretty_print;
	_pretty_print = on;
	return prev;
}
void z_json_stream::write_std(const std::string& x)
{
	_out.write_std(x);
}

z_status z_json_stream::write(const char* data, size_t len)
{
	return _out.write(data,len);
}

void z_json_stream::reset(bool pretty_print)
{

	_depth = 0;
	_need_comma = false;
	_pretty_print = pretty_print;

}

// remember to turn off pretty print for simple array
void z_json_stream::obj_array_start()
{
	_need_comma = false;
	if (_pretty_print) indent(_depth);

	_out << '[';
	if (_pretty_print) _out << '\n';
	_depth++;
}

void z_json_stream::obj_array_end()
{
	_depth--;
	if (_pretty_print)
	{
		*this << '\n';
		indent(_depth);
	}
	_out << ']';
	_need_comma = true;
}
// remember to turn off pretty print for simple array

void z_json_stream::array_start()
{
	if (_need_comma)
		_out << ',';
	_need_comma = false;
	if (_pretty_print)
	{
		
		indent(_depth);
	}
	_out << '[';
}

void z_json_stream::array_end()
{
	_out << ']';
	_need_comma = true;

}
void z_json_stream::obj_val_start(ctext name)
{
	key(name);
	obj_start();

}

void z_json_stream::obj_start()
{
	if (_need_comma)
		_out << ',';
	_need_comma = false;

	_out << '{';
	if (_pretty_print) _out << '\n';
	_depth++;

}

void z_json_stream::obj_end()
{
	_depth--;
	if (_pretty_print)
	{
		*this << '\n';
		indent(_depth);
	}
	_out << '}';
	_need_comma = true;
}
void z_json_stream::val_end()
{
	_need_comma = true;

}
void z_json_stream::val(ctext val)
{
	//z_string esc = escape(val);
	// cannot escape, because val may be in quotes.?
	_out << val;
	//_out << '\"' << esc.copy_escaped(val) << '\"';
	_need_comma = true;

}
void z_json_stream::val_str_array(z_strlist& list)
{
	array_start();
	for (auto i : list)
	{
		*this% i;
	}
	array_end();

}
void z_json_stream::keyval_int(ctext name, I64 val)
{
	key(name);
	_out << val;
	_need_comma = true;
}
void z_json_stream::key_bool(ctext name, bool val)
{
    key(name);
    _out << val;
    _need_comma = true;

}

void z_json_stream::keyval(ctext name,ctext val)
{
	key(name);
	z_string esc = escape(val);
	_out << '\"' << esc << '\"';
	_need_comma = true;
}
void z_json_stream::key(ctext name)
{
	if (_need_comma)
	{
		*this << ',';
		if (_pretty_print) *this << '\n';
	}
	if (_pretty_print)
	{
		indent(_depth);
	}
	_out << '\"' << name << "\" : ";
	_need_comma = false;

}

z_json_stream & z_json_stream::operator % (const z_string & x)
{
	z_string esc = escape(x);
	if (_need_comma)
		_out << ',';
	_out << '\"' << esc << '\"';
	_need_comma = true;

	return *this;

}
z_json_stream & z_json_stream::operator % (const char* x)
{
	z_string esc = escape(x);

	if (_need_comma)
		_out << ',';
	_out << '\"' << esc << '\"';
	_need_comma = true;
	return *this;

}
z_json_stream& z_json_stream::operator % (U32 x)
{
	if (_need_comma)
		_out << ',';
	_out << x;
	_need_comma = true;
	return *this;
}
z_json_stream & z_json_stream::operator % (int x)
{
	if (_need_comma)
		_out << ',';
	_out <<  x;
	_need_comma = true;
	return *this;
}
z_json_stream & z_json_stream::operator % (double x)
{
	if (_need_comma)
		_out << ',';
	_out << x;
	_need_comma = true;
	return *this;
}

z_json_stream & z_json_stream::operator % (bool x)
{
	if (_need_comma)
		_out << ',';
	if(x)
		_out << "true";
	else
		_out << "false";

	_need_comma = true;
	return *this;
}
z_json_stream & z_json_stream::operator % (int64_t x)
{
	if (_need_comma)
		_out << ',';
	_out << x;
	_need_comma = true;
	return *this;
}
