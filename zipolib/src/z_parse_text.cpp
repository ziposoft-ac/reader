#include "pch.h"

#include "zipolib/z_parse_text.h"


class char_sets
{
public:
	char_sets() :
		letters("_A-Za-z"),
		hexdigits("0-9A-Fa-f"),
		digits("0-9"),
		integer(digits+'-'),
		floating(digits + "-."),
		identchars(letters + digits),
		scoped_identchars(identchars + ':'),
		path_string(identchars + "\\/.:" + '-'),
		white_space(" \t\r\n")
	{ }
	cset letters;
	cset hexdigits;
	cset digits;
	cset integer;
	cset floating;
	cset identchars;
	cset scoped_identchars;
	cset path_string;
	cset white_space;


};

const char_sets& get_char_sets()
{
	static const char_sets sets;
	return sets;

}

//________________________________________________________________
//
// INIT
//________________________________________________________________

zp_text_parser::zp_text_parser()
{
	set_source(0,0);
	_options.as_u32=0;
}
zp_text_parser::zp_text_parser(ctext code,size_t len) 
{
	set_source(code,len);
	_options.as_u32=0;
}

z_status zp_text_parser::check_status(z_status status)
{
	if(status>zs_eof)
	{
		int breakpoint;
		breakpoint=1;
	}
	if(status==zs_eof)
	{
		int breakpoint;
		breakpoint=1;
	}
	return status;
}

void zp_text_parser::set_source(z_string& s)
{
    n_newlines=0;
    _index_under_test=0;
    _match_start=0;
    _start=	s.c_str();
    set_index(_start);
    _end=_start+s.size();
}
void zp_text_parser::set_source(const char* code,size_t len)
{
	n_newlines=0;
	_index_under_test=0;
	_match_start=0;
	if(len==(size_t)-1)
	{
		if(code)
			len=strlen(code);
	}

	set_index(code);
	_start=	code;
	_end=_start+len;
}
//________________________________________________________________
//
// UTIL
//________________________________________________________________

bool zp_text_parser::eob(char* i) { return (i>=_end);}
bool zp_text_parser::eob() 
{ 
	return ((_index_current==0)||(_index_current>=_end)||(*_index_current)==0);
}
char zp_text_parser::current_ch() 
{ 
	//	_index_under_test= _index_current;
	return *_index_current; 
}

void zp_text_parser::index_reset()
{
	_index_current=_start;
}
void zp_text_parser::set_option_count_lines()
{
	_options.count_lines=1;

}
void zp_text_parser::set_option_ignore_whitespace()
{
	_options.ignore_tabs=1;
	_options.ignore_space=1;
	_options.ignore_newline=1;

}
			  

z_status zp_text_parser::advance(size_t count)
{
	if((!_index_current) ||	  ( _index_current+count>_end))
		return check_status(zs_eof);	
	if(_options.count_lines==0)
	{

		_index_current+=count;
		return zs_ok;
	}


	/* if we needed to count newlines.*/

	while(count)
	{
		if(*_index_current=='\n')
		{
			n_newlines++;
		}
		_index_current++;
		count--;
	}
	
	return zs_ok;
}


char zp_text_parser::inc()
{
	if (advance(1)) return 0;
	return current_ch();

}
z_status zp_text_parser::skip_ws()
{
	z_status status=zs_no_match;
	while(1)
	{
		if(eob())
			return check_status(zs_eof);
		const char c= current_ch();
		if((c==' ')||
			(c=='\t')||
			(c=='\n')||
			(c=='\r'))
		{
			inc();
			status=zs_matched;
			continue;
		}
		return status;
	}
}

z_status zp_text_parser::start_test()
{
	if(eob()) 
		return check_status(zs_eof);
	_match_start=get_index();
	set_index_under_test(get_index());
	_match_end=(ctext)-1;
	return zs_matched;

}

void zp_text_parser::print_context(z_stream& s)
{
	z_string data;
	if(!_index_under_test)
		return;
	size_t arrow=debug(data,_index_under_test,12,12);
	s << data << '\n';
	size_t i;
	for(i=0;i<arrow;i++)
	{
		s <<".";
	}
	s<< "^\n";
}
size_t zp_text_parser::debug(z_string &out,ctext pbuff,size_t before,size_t after)
{
	if(!pbuff)
		pbuff=get_index();

	const char* c=pbuff-before;
	if(c<_start)
	{
		c=_start;
		before=pbuff-_start;
	}
	const char* end=pbuff+after;
	if(end>_end)
	{
		end=_end;
		after=_end-pbuff;
	}
	while(c<end)
	{
		bool escaped=true;
		switch(*c)
		{
		case '\n':
			out << "\\n";
			break;
		case '\r':
			out << "\\r";
			break;
		case '\\':
			out << "\\";
			break;
		case '\t':
			out << "\\t";
			break;
		case ' ':
			escaped=false;
			out << " ";
			break;
		default:

			escaped=false;
			out<<*c;
			break;
		}
		if(escaped)
		{
			if(c<pbuff)
				before++;
			if(c>pbuff)
				after++;
		}
		c++;
	}
	return before;

}
//________________________________________________________________
//
// INTERNAL TESTS
//________________________________________________________________
z_status zp_text_parser::_test_char(char c)
{
	if(eob()) 
		return check_status(zs_eof);
	if(current_ch()== c)
	{
		inc();
		return zs_matched;
	}
	return zs_no_match;
}
z_status zp_text_parser::_test_end_char(char c) //  ~>(char) 
{
	while(!eob()) 
	{
		if(current_ch() == c)
		{
			inc();
			return zs_matched;
		}
		inc();
	}
	return check_status(zs_eof);
}

//________________________________________________________________
//
// EXTERNAL TESTS
//________________________________________________________________
z_status zp_text_parser::test_chars(const cset &set)
{
	z_status status;
	if((status=start_test())) return status;


	if(current_ch()& set)
	{
		inc();
		return zs_matched;
	}
	return zs_no_match;
}
z_status zp_text_parser::test_to_eob()
{
	z_status status;
	if((status=start_test())) return status;
	set_index(_end);
	return zs_matched;
}
z_status zp_text_parser::_peek_next_char(char c)
{
	//z_status status;
 	if( _index_current==_end)
		return zs_eof;
	if(*(_index_current+1) == c)
	{
		return zs_matched;
	}
	return zs_no_match;
}
z_status zp_text_parser::test_char(char c)
{
	z_status status;
	if((status=start_test())) return status;

	if(current_ch() == c)
	{
		inc();
		return zs_matched;
	}
	return zs_no_match;
}
z_status zp_text_parser::test_file_path()
{
	return test_cset(get_char_sets().path_string);
}



size_t zp_text_parser::get_index_offset()
{
	return _index_current-_start;
}
z_status zp_text_parser::test_not_whitespace()
{
	return test_not_cset(get_char_sets().white_space);
}
z_status zp_text_parser::test_whitespace()
{
	return test_cset(get_char_sets().white_space);
}
z_status zp_text_parser::test_floating_point()
{
	z_status status;
	if ((status = start_test()))
		return status;
	ctext start = get_index();
	status = _test_cset(get_char_sets().integer);
	if (status == zs_matched)
	{
		status = _test_char('.');
		if (status == zs_matched)
		{
			status = _test_cset(get_char_sets().integer);
		}
	}

	if (status)
	{
		set_index(start);
	}
	return status;
}
z_status zp_text_parser::test_integer()
{
	return test_cset(get_char_sets().integer);
}
z_status zp_text_parser::test_integer_get(U32 &i)
{
    z_status status;
    status=test_cset(get_char_sets().integer);
    if(status) return status;
    z_string str;
    get_match(str);
    i=str.get_int_val();
    return zs_ok;



}
z_status zp_text_parser::test_hex_integer()
{

	z_status status;
	if ((status = start_test()))
		return status;
	ctext start = get_index();
	status = test_string("0x",2);
	if (status == zs_matched)
	{
		status = _test_cset(get_char_sets().hexdigits);
	}

	if (status)
	{
		set_index(start);
	}
	return status;


	return test_cset(get_char_sets().hexdigits);
}
z_status zp_text_parser::test_any_identifier_scoped()
{
	return test_cset(get_char_sets().scoped_identchars);
}
z_status zp_text_parser::test_any_identifier()
{
	return test_cset(get_char_sets().identchars);
}
z_status zp_text_parser::test_identifier(const char* str)
{
	z_status status;
	if((status=start_test())) return status;

	size_t match_len=0;
	size_t len=_end-get_index();
	size_t test_len=strlen(str);
	if (test_len> len) 
		return zs_no_match;
	while(match_len<len)
	{
		if(_index_current[match_len] & get_char_sets().identchars)
		{
			match_len++;
		}
		else
		{
			break;
		}

	}
	if (test_len== match_len) 
	{
		if(memcmp(str,get_index(),test_len)==0) 
		{
			advance(match_len);
			return zs_matched;
		}
	}
	return zs_no_match;
}

z_status zp_text_parser::test_single_quoted_string()
{
	z_status status;
	if((status=start_test())) 
		return status;

	if((status=_test_char('\''))) 
		return status;
	_match_start=get_index();
	if((_test_end_char('\''))) 
		return check_status(zs_syntax_error);
	_match_end=get_index()-1;
	return zs_matched;
}
z_status zp_text_parser::test_not_single_quoted_string(const void *dummy)
{
	z_status status;
	if((status=start_test())) 
		return status;

	if((status=_test_char('^'))) 
		return status;
	//inc();
	if((status=_test_char('\''))) 
		return check_status(zs_syntax_error);
	_match_start=get_index();
	if((_test_end_char('\''))) 
		return check_status(zs_syntax_error);
	_match_end=get_index()-1;
	return zs_matched;
}


z_status zp_text_parser::test_code_string()
{
	z_status status;
	bool escape=false;
	if((status=start_test())) 
		return status;

	if((status=_test_char('\"'))) 
		return status;
	_match_start=get_index();

	while(!eob())
	{
		if(_test_char('\\')==zs_matched)
			escape=true;

		if(_test_char('\"')==zs_matched)
		{
			if(escape)
			{
				escape=false;
				continue;
			}
			_match_end=get_index()-1;
			return zs_matched;
		}
		escape=false;
		inc();
	}
	return check_status(zs_syntax_error);
}
z_status zp_text_parser::_test_cset(const cset &set)
{

	if (!(*_index_current & set))
		return zs_no_match;
	_index_current++;
	while (_index_current<_end)
	{
		if (!(*_index_current & set))
			break;
		_index_current++;
	}
	return  zs_matched;
}

z_status zp_text_parser::test_cset(const cset &set)
{
	z_status status;
	if((status=start_test())) return status;
	return  _test_cset(set);
}
z_status zp_text_parser::test_not_cset(const cset &set)
{
	z_status status;
	if ((status = start_test())) return status;

	if (*_index_current & set)
		return zs_no_match;
	_index_current++;
	while (_index_current<_end)
	{
		if (*_index_current & set)
			break;
		_index_current++;
	}
	return  zs_matched;
}


z_status zp_text_parser::test_end_char(char c) //  ~>(char) 
{
	z_status status;
	if((status=start_test())) 
		return status;

	return _test_end_char(c);
}


z_status zp_text_parser::test_not_char(char c) //  ~^(char)  ~^\n
{
	z_status status;
	if((status=start_test())) return status;

	while(!eob()) 
	{
		if(current_ch() == c)
		{
			return zs_matched;
		}
		inc();
	}
	return zs_matched;
}
z_status zp_text_parser::test_any_chars(size_t n)// #5
{
	z_status status;
	if((status=start_test())) return status;

	size_t len_left=_end-get_index();
	if (len_left< n) 
		return zs_no_match;

	advance(n);
	return zs_matched;
}
z_status zp_text_parser::test_end_string(const char* str)
{
	z_status status;
	if((status=start_test())) return status;
	int matched=0;
	size_t match_len=strlen(str);
	while(!eob())
	{
		size_t len=_end-get_index();
		if (match_len> len) 
			return zs_matched;
		if(memcmp(str,get_index(),match_len)==0)
		{
			if(matched)
			{
				advance(match_len);//Skip end str
				return zs_matched;
			}
			return zs_no_match;

		}
		matched++;
		inc();
	}
	return zs_matched;
}

z_status zp_text_parser::test_not_string(const char* str,size_t match_len)
{
	z_status status;
	if((status=start_test())) return status;
	int matched=0;

	while(!eob())
	{
		size_t len=_end-get_index();
		if (match_len> len) 
			return zs_matched;
		if(memcmp(str,get_index(),match_len)==0)
		{
			if(matched)
				return zs_matched;
			return zs_no_match;

		}
		matched++;
		inc();
	}
	return zs_matched;
}
z_status zp_text_parser::test_string(const char* str,size_t match_len)
{
	z_status status;
	if((status=start_test())) 
		return status;

	size_t len=_end-get_index();
	if (match_len> len) 
		return check_status(zs_eof);
	if(memcmp(str,get_index(),match_len)==0)
	{
		advance(match_len);		
		return zs_matched;
	}
	return zs_no_match;
}

z_status zp_text_parser::test_string(const char* str)
{
	size_t match_len=strlen(str);
	return test_string(str,match_len);
}

//________________________________________________________________
//
// RETURNING MATCHES
//________________________________________________________________

void zp_text_parser::get_match_from(ctext match_start, z_string& s)
{
	size_t len;
	if (_match_end == (ctext)-1)	 //there is some voodoo happening here.
		_match_end = get_index();

	len = _match_end - match_start;

	s.assign(match_start, len);
}
void zp_text_parser::get_match(z_string& s)
{
	ctext start;
	size_t len;
	get_match(start,len);
	if(!start)
		return;
	s.assign(start,len);
}
void zp_text_parser::get_match(ctext& match_start, size_t& len)
{
	match_start = _match_start;
	if (_match_end == (ctext)-1)	 //there is some voodoo happening here.
		_match_end = get_index();

	len = _match_end - match_start;
}
ctext zp_text_parser::get_match( ctext& match_end)
{
	if (_match_end == (ctext)-1)	 //there is some voodoo happening here.
		_match_end = get_index();

	match_end = _match_end;
	return _match_start;
}



z_status zp_text_parser::ft_letters  (const void* dummy) 
{
	return test_cset(get_char_sets().letters);
}

z_status zp_text_parser::ft_digits  (const void* dummy) 
{
	return test_cset(get_char_sets().digits);
}

z_status zp_text_parser::ft_single_quoted_string(const void* dummy)
{
	return test_single_quoted_string();
}
z_status zp_text_parser::ft_file_path(const void* dummy)
{
	return test_file_path();
}

z_status zp_text_parser::ft_any_identifier(const void* dummy)
{
	return test_any_identifier();
}
z_status zp_text_parser::ft_scoped_identchars(const void* dummy)
{
	return test_cset(get_char_sets().scoped_identchars);
}
z_status zp_text_parser::ft_to_eol(const void* dummy)
{
	return  test_not_char('\n');
}

z_status zp_text_parser::ft_test_identifier(const void* str)
{
	return  test_identifier((ctext)str);

}
z_status zp_text_parser::ft_test_char(const void* c)
{
	return test_char((char)( (size_t)c & 0xFF));

}
