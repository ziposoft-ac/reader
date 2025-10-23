#ifndef terminal_h
#define terminal_h
#include "zipolib/zipo.h"
#include "zipolib/z_strlist.h"


namespace term_keys
{
enum z_enum_key
{
	key_alpha,
	key_right,
	key_left,
	key_up,
	key_down,
	key_esc,
	key_enter,
	key_home,
	key_pageup,
	key_page_down,
	key_insert,
	key_delete,
	key_end,
	key_ctrl_C,
	key_back,
	key_tab,
	key_unknown,
	key_max
};
struct key_def
{
	z_enum_key type;
	ctext name;
	uint length;
	ctext code;
};
}


/*

ToDO - this is terrible clean this up

*/
class z_terminal_os_specific;

class z_terminal 
{
	z_terminal_os_specific* _os;

protected:
	//Input buffer
	char *_pBuffer=0;
	uint _buffSize=0;
	uint _buffNext=0;
	uint _buffEnd=0;

	size_t _key_map_count;
	const term_keys::key_def* _key_map;
	size_t _proccessId;
public:	
	z_terminal();
	virtual ~z_terminal();

	enum term_type { tt_vt100=0, tt_windows = 1, tt_linux = 2, tt_max };
	char _char_back;
	bool _debug;
	bool _opened;
	//Cursor 
	uint cur_x;
	uint cur_y;
	char  BuffPeekChar(size_t index)
	{
		Z_ASSERT( _pBuffer);
		size_t x=index+_buffNext;
		if (x>=_buffSize)
			x=x-_buffSize;
		return _pBuffer[x];
	}
	void  BuffAdvance(uint index)
	{
		_buffNext+=index;
		if(_buffNext>=_buffSize) _buffNext-=_buffSize;
	}
	void  BuffAddChar(char c)
	{
		//Z_ASSERT( _pBuffer);
		//zout.putf("%c %02x\r\n",c,c);
		if(!  _pBuffer) 
			return; //TODO
		_pBuffer[_buffEnd]=c;
		_buffEnd++;
		if(_buffEnd >=_buffSize) _buffEnd=0;
	}
	z_status  WaitForKey();
	size_t  BuffGetCount() ;

	void ForceKey(char c);
	void ForceCtrlC();
    void run_debug();
	virtual bool GetKey(term_keys::z_enum_key& key,char &c);
	virtual void curGotoXY(uint x,uint y);
	virtual void curLeft(uint x);
	virtual void curRight(uint x);
	virtual void GetXY();
	virtual void Close();
	virtual bool terminal_open();
	int set_key_map(term_type map);

};

class z_console_base : public z_terminal
{

	bool _console_running = false;

	uint index;
	uint cur_start;
	uint len;
	bool insertmode;
	term_keys::z_enum_key _key;
	term_keys::z_enum_key _prev_key;
	uint _history_index;
	z_string _partial;
	uint get_index();
	void AppendChar(char ch);
	void RedrawLine(int blanks = 0);
	void InsertChar(char ch);
	void OverwriteChar(char ch);
	void hChar(char ch);

	void clear_line();
	void inc_history(uint i);

protected:
	void reset_line();
	z_strlist _auto_tab;
	bool _tab_mode;
	uint  _tab_count;
	uint  _tab_index;
	uint  _tab_mode_line_index;
	void trim_line_to(uint trim_point);
	uint get_line_length();
	void output(ctext text);

	z_string _buffer;

	virtual z_status ExecuteLine(ctext text);
	virtual void LogInput(ctext text) {};
	virtual void OnEnter();
	virtual void OnTab();
	virtual void OnUp();
	virtual void OnDown();
	virtual void OnDoubleBack();
	virtual void put_prompt() {}

public:
	z_string  _param_path;
	z_strlist _history;

	bool is_console_running() { return _console_running; }

	z_console_base();
	virtual z_status run();
	virtual void quit();

};


#endif
