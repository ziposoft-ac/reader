#include "pch.h"
#include "zipolib/z_terminal.h"

using namespace term_keys;

const key_def km_winconsole[]=
{ 
	{key_back		,"back",	1,	"\x008"},

	{key_right		,"right",	2,	"\x0e0\x04d"},
	{key_left		,"left",	2,	"\x0e0\x04b"},
	{key_up			,"up",		2,	"\x0e0\x048"},
	{key_down		,"down",	2,	"\x0e0\x050"},
	{key_home		,"home",	2,	"\x0e0\x047"},
	{key_end		,"end",	    2,	"\x0e0\x04f"},
	{key_pageup		,"pageup",	2,	"\x0e0\x049"},
	{key_page_down	,"page_down", 2,	"\x0e0\x051"},
	{key_insert		,"insert",	2,	"\x0e0\x052"},
	{key_delete		,"delete",	2,	"\x0e0\x053"},
	{key_enter		,"enter",	1,	"\x00d"},
	{key_tab		,"tab",		1,	"\x009"},
	{key_ctrl_C		,"break",	1,	"\x003"},
	{key_esc		,"esc",		1,	"\x01b"},
	{key_alpha		,"alpha",	0,	"?"},
	{key_unknown	,"unknown",	0,	"?"}
};
const size_t km_winconsole_count= sizeof(km_winconsole)/sizeof(key_def);

const key_def km_VT100[]=
{ 
	{key_back		,"back",	1,	"\x008"},



	{key_right		,"right",	3,   "\x01b\x05b\x043"},
	{key_left		,"left",	3,	 "\x01b\x05b\x044"},
	{key_up			,"up",		3,   "\x01b\x05b\x041"},
	{key_down		,"down",	3,   "\x01b\x05b\x042"},
	{key_insert		,"insert",	3,"\x01b\x05b\x031"},
	{key_home		,"home",	3,"\x01b\x05b\x032"},
	{key_pageup		,"pageup",	3,"\x01b\x05b\x033"},
	{key_delete		,"delete",	3,"\x01b\x05b\x034"},
	{key_end		,"end",	    3,"\x01b\x05b\x035"},
	{key_page_down	,"pagedown	", 3,"\x01b\x05b\x036"},
	{key_enter		,"enter",	1,	"\x00a"},

	{key_tab		,"tab",		1,	"\x009"},
	{key_ctrl_C		,"break",	1,	"\x003"},
	{key_esc		,"esc",		1,	"\x01b"},
	{key_alpha		,"alpha",	0,	"?"},
	{key_unknown	,"unknown",	0,	"?"}
};
const key_def km_linux[] =
{
        { key_back		,"back",	1,	"\x008" },
        { key_back		,"back",	1,	"\x07f" },



	{ key_right		,"right",	3,   "\x01b\x05b\x043" },
	{ key_left		,"left",	3,	 "\x01b\x05b\x044" },
	{ key_up			,"up",		3,   "\x01b\x05b\x041" },
	{ key_down		,"down",	3,   "\x01b\x05b\x042" },
	{ key_pageup		,"pageup",	4,"\x01b\x05b\x035\x07e" },
	{ key_page_down		,"pagedown",	4,"\x01b\x05b\x036\x07e" },
	{ key_insert		,"insert",	4,"\x01b\x05b\x032\x07e" },
	{ key_delete		,"delete",	4,"\x01b\x05b\x033\x07e" },

	{ key_home		,"home",	4,"\x01b\x05b\x031\x07e" },
	{ key_end		,"end",	    4,"\x01b\x05b\x034\x07e" },
	{ key_home		,"home",	3,"\x01b\x04f\x048" },
	{ key_end		,"end",	    3,"\x01b\x04f\x046" },
    { key_home		,"home",	3,"\x01b\x05b\x048" },
    { key_end		,"end",	    3,"\x01b\x05b\x046" },

    { key_enter		,"enter",	1,	"\x00a" },
    { key_enter		,"enter",	1,	"\x00d" },

	{ key_tab		,"tab",		1,	"\x009" },
	{ key_ctrl_C		,"break",	1,	"\x003" },
	{ key_esc		,"esc",		1,	"\x01b" },
	{ key_alpha		,"alpha",	0,	"?" },
	{ key_unknown	,"unknown",	0,	"?" }
};
struct key_map
{
	z_terminal::term_type id;
	const key_def *map;
	size_t size;
};
const key_map key_maps[]=
{
	//keep in order, indexed by enum
	{ z_terminal::tt_vt100,km_VT100,sizeof(km_VT100) / sizeof(key_def)},
	{ z_terminal::tt_windows,km_winconsole,km_winconsole_count },
	{ z_terminal::tt_linux,km_linux,sizeof(km_linux) / sizeof(key_def) },
};

int z_terminal::set_key_map(term_type map)
{
	if(map>=tt_max)
		return -1; //error
	_key_map=key_maps[map].map;
	_key_map_count=key_maps[map].size;

	return 0;
}

const key_def oldkeymap[]=
{ 
	{key_back		,"back",	1,	"\x008"},
#if 1
#ifdef UNIX
	{key_page_down	,"page_down	", 4,"\x01b\x05b\x036\x07e"},
	{key_right		,"right",	3,   "\x01b\x05b\x043"},
	{key_left		,"left",	3,	 "\x01b\x05b\x044"},
	{key_up			,"up",		3,   "\x01b\x05b\x041"},
	{key_down		,"down",	3,   "\x01b\x05b\x042"},
#endif
#ifdef LINUX
	{key_pageup		,"pageup",	4,"\x01b\x05b\x035\x07e"},
	{key_insert		,"insert",	4,"\x01b\x05b\x032\x07e"},
	{key_delete		,"delete",	4,"\x01b\x05b\x033\x07e"},
	{key_home		,"home",	3,"\x01b\x05b\x048"},
	{key_end		,"end",	    3,"\x01b\x05b\x049"},

#endif
#ifdef LYNX
	{key_pageup		,"pageup",	4,"\x01b\x05b\x033\x07e"},
	{key_insert		,"insert",	4,"\x01b\x05b\x031\x07e"},
	{key_delete		,"delete",	4,"\x01b\x05b\x034\x07e"},
	{key_home		,"home",	3,"\x01b\x05b\x032"},
	{key_end		,"end",	    3,"\x01b\x05b\x035"},
#endif
#endif

#ifdef BUILD_VSTUDIO
	{key_right		,"right",	2,	"\x0e0\x04d"},
	{key_left		,"left",	2,	"\x0e0\x04b"},
	{key_up			,"up",		2,	"\x0e0\x048"},
	{key_down		,"down",	2,	"\x0e0\x050"},
	{key_home		,"home",	2,	"\x0e0\x047"},
	{key_end		,"end",	    2,	"\x0e0\x04f"},
	{key_pageup		,"pageup",	2,	"\x0e0\x049"},
	{key_page_down	,"page_down", 2,	"\x0e0\x051"},
	{key_insert		,"insert",	2,	"\x0e0\x052"},
	{key_delete		,"delete",	2,	"\x0e0\x053"},
	{key_enter		,"enter",	1,	"\x00d"},

	#else
	{key_pageup		,"pageup",	4,"\x01b\x05b\x035\x07e"},
	{key_insert		,"insert",	4,"\x01b\x05b\x032\x07e"},
	{key_delete		,"delete",	4,"\x01b\x05b\x033\x07e"},
	{key_home		,"home",	3,"\x01b\x05b\x048"},
	{key_end		,"end",	    3,"\x01b\x05b\x049"},
	{key_enter		,"enter",	1,	"\x00a"},

#endif
	{key_tab		,"tab",		1,	"\x009"},
	{key_ctrl_C		,"break",	1,	"\x003"},
	{key_esc		,"esc",		1,	"\x01b"},
	{key_alpha		,"alpha",	0,	"?"},
	{key_unknown	,"unknown",	0,	"?"}
};





