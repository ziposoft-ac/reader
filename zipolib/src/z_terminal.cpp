#include "pch.h"

#include "zipolib/z_error.h"
#include "zipolib/z_terminal.h"
#include "zipolib/z_exception.h"

using namespace term_keys;


#ifdef BUILD_VSTUDIO
#include <wincon.h>
#include <conio.h>
#endif
#ifdef BUILD_MINGW
#include <winbase.h>
#include <wincon.h>
#include <conio.h>

#else
#ifdef UNIX
#include <signal.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
void CatchSigs();
#endif
#endif


class z_terminal_os_specific
{
public:

#ifdef BUILD_VSTUDIO
    HANDLE _hStdout, _hStdin;
	//HANDLE _hConsoleOutput;
	CONSOLE_SCREEN_BUFFER_INFO _csbiInfo;
	z_terminal_os_specific()
	{
		_hStdout = NULL;
		_hStdin = NULL;
	}
#else
    z_terminal_os_specific()
    {
    }
    virtual ~z_terminal_os_specific()
    {
        if(_raw_mode)
            tcsetattr(0, TCSANOW, &_term_original);
    }
    struct termios _term_original;
    struct termios _term_wait;
    struct termios _term_raw;
    bool _raw_mode=false;
    int get_resp(char* buff, char token)
    {
        int n = 0;
        char c = 0;
        // This is terrible. I don't know how it is working.

        while (getch(c))
        {
            getch(c);

            buff[n] = c;
            if(++n > 14)
                break;
            if(c==token)
                break;


        }
        buff[n] = 0;
        return 0;
    }
    int close()
    {
        return 0;
    }
    int open(z_terminal *term)
    {
        tcgetattr(0, &_term_original);
        tcgetattr(0, &_term_wait);
        tcgetattr(0, &_term_raw);
        _term_wait.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);

       // _term_wait.c_lflag &= ~(ICANON|ECHO);
        //_term_raw.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP   |INLCR|IGNCR|ICRNL|IXON);

        //_term_raw.c_oflag &= ~OPOST;
        _term_raw.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);

        //_term_raw.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
       // _term_raw.c_lflag &= ~(ECHO|ECHONL|ICANON);
       // tcsetattr(0, TCSANOW, &_term_raw);
        term->_char_back = _term_original.c_cc[VERASE];
        _raw_mode=true;
        return 0;
    }
    int kbhit()
    {
        struct timeval tv = { 0L, 0L };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        return select(1, &fds, NULL, NULL, &tv);
    }
    int getch(char& c)
    {
        int r;
        if ((r = read(0, &c, sizeof(c))) != 1) {
            return 0;
        } else {
            return 1;
        }
    }
    int mode_normal()
    {
        int result = 0;
        result = tcsetattr(fileno(stdin), TCSANOW, &_term_original);
        return(result);
    }
    int set_raw()
    {
        int result = 0;
        result = tcsetattr(0, TCSANOW, &_term_raw);
        return(result);
    }
    int set_wait()
    {
        int result = 0;
        result = tcsetattr(0, TCSANOW, &_term_wait);
        return(result);
    }
#endif
};


z_terminal::z_terminal()
{
    _pBuffer=0;
#ifdef WIN32
    set_key_map(tt_windows);
#else
    set_key_map(tt_linux);
#endif
    _opened=false;
    _debug = false;
    _char_back = 0;
    _os = z_new z_terminal_os_specific();
};
z_terminal::~z_terminal()
{
    if (_pBuffer)
        delete []_pBuffer;
    delete _os;
};

void z_terminal::run_debug()
{
    terminal_open();
    term_keys::z_enum_key key;
    char c;
    _debug=true;
    while(1)
    {
        GetKey(key, c);
        if(key==key_ctrl_C)
            break;
    }
}
size_t  z_terminal::BuffGetCount()
{
    if (_buffNext>_buffEnd)
    {
        return (_buffSize-_buffNext+_buffEnd);
    }
    return (_buffEnd-_buffNext);
}
bool z_terminal::terminal_open()
{
    if(	_opened)
        return true;
#ifdef WIN32
    _proccessId = GetCurrentProcessId();
	_os->_hStdin = GetStdHandle(STD_INPUT_HANDLE);
	_os->_hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
#ifdef UNIX
    _os->open(this);
    setvbuf(stdout, NULL, _IONBF, 0);
#endif

    _buffSize=0x40;
    _pBuffer=z_new char[_buffSize];
    _buffNext=0;
    _buffEnd=0;
    GetXY();
    _opened=true;
    return(0);

}
void z_terminal::Close()
{
    _opened=false;
}

bool z_terminal::GetKey(z_enum_key& key,char &c)
{

    size_t i,j;
    size_t count;
    bool match=false;
    key=key_alpha;
    if(!_pBuffer){
        terminal_open();
        //return false; //not open?
    }

    count=BuffGetCount();
    if(!count)
        WaitForKey();
    if(_debug)
    {
        printf("key:");
        for(j=0;j<BuffGetCount();j++)
            printf("%02x ",BuffPeekChar(j));
        printf("\r\n");
    }

    for(i=0;i<_key_map_count;i++)
    {
        if(_key_map[i].type==key_alpha)
        {
            c=BuffPeekChar(0);
            BuffAdvance(1);
            if((c>=0x20)&&(c<=0x7e))
            {

                key=key_alpha;
                return true;
            }
            key=key_unknown;
            return false;
        }
        if((i==0)&&(BuffPeekChar(0)== _char_back))
        {
            match=true;
            break;
        }
        if(_key_map[i].length==BuffGetCount())
        {
            match=true;
            for(j=0;j<_key_map[i].length;j++)
            {
                if(_key_map[i].code[j]!=BuffPeekChar(j))
                {
                    match=false;
                    break;
                }
            }
        }
        if(match) break;
    }
    if(match)
    {
        if(_debug)
        {
            printf("%s\r\n",_key_map[i].name);
        }
        key=_key_map[i].type;
        BuffAdvance(_key_map[i].length);
        return true;
    }

    return false;
}





////////////////////////////////////////////////////////////
// WIN32 stuff
////////////////////////////////////////////////////////////
#ifdef WIN32
void z_terminal::ForceKey(char c)
{
	DWORD num;
	INPUT_RECORD record;
	record.EventType = KEY_EVENT;
	record.Event.KeyEvent.bKeyDown = TRUE;
	record.Event.KeyEvent.wRepeatCount = 1;
	record.Event.KeyEvent.uChar.AsciiChar = c;
	record.Event.KeyEvent.wVirtualKeyCode = 0x03;
	record.Event.KeyEvent.wVirtualScanCode = 0;
	record.Event.KeyEvent.dwControlKeyState = 0;

	WriteConsoleInput(_os->_hStdin, &record, 1, &num);


}
void z_terminal::ForceCtrlC()
{
	GenerateConsoleCtrlEvent(CTRL_BREAK_EVENT, 0);
	//ExitProcess(-1);

}

void z_terminal::GetXY()
{
	BOOL b=GetConsoleScreenBufferInfo(_os->_hStdout,&_os->_csbiInfo);
	if(b)
	{
		cur_x= _os->_csbiInfo.dwCursorPosition.X;
		cur_y= _os->_csbiInfo.dwCursorPosition.Y;
	}
	else
	{
		//PrintWin32Error();
	}

}
void z_terminal::curGotoXY(uint x,uint y)
{
	COORD coord;
	coord.X=(SHORT)x;
	coord.Y= (SHORT)y;
	SetConsoleCursorPosition(_os->_hStdout,coord);
}
void z_terminal::curLeft(uint x)
{
	cur_x=cur_x-x;
	if(cur_x<0) cur_x=0;
	curGotoXY(cur_x,cur_y);
}
void z_terminal::curRight(uint x)
{
	cur_x=cur_x+x;
	curGotoXY(cur_x,cur_y);
}

void z_terminal::WaitForKey()
{
	BuffAddChar(_getch());
	while (_kbhit())
	{
		BuffAddChar(_getch());
	}
}
#else
////////////////////////////////////////////////////////////
// UNIX stuff
////////////////////////////////////////////////////////////
void z_terminal::ForceKey(char c)
{



}
void z_terminal::ForceCtrlC()
{


}

z_status z_terminal::WaitForKey()
{
    int i;
    char c=0;
    _os->set_raw();
    _os->getch(c);

    BuffAddChar(c);
    if(c==0x1b)
    {

        while(_os->kbhit())
        {

            _os->getch(c);
            if (c == EOF)
                break;
            BuffAddChar(c);
        }
    }
    _os->mode_normal();

    return zs_ok;

}




void z_terminal::GetXY()
{
    char buf[30]={0};
    int ret, i;
    char ch;
    _os->set_raw();

    write(1, "\033[6n", 4);

    for( i = 0, ch = 0; ch != 'R'; i++ )
    {
        ret = read(0, &ch, 1);
        if ( !ret ) {
            break;
        }
        buf[i] = ch;
        //printf("buf[%d]: \t%c \t%d\n", i, ch, ch);
    }
    if(i>4)
    {
        sscanf(buf+1,"[%d;%dR",&cur_y,&cur_x);
        cur_x--;
    }
    _os->mode_normal();

}

void z_terminal::curGotoXY(uint x,uint y)
{
    //<ESC>[{ROW};{COLUMN}f
    //z_string x;
    //x.format()
    printf("\x01b[%d;%df",y,x+1);
}
void z_terminal::curLeft(uint x)
{
    //<ESC>[{ROW};{COLUMN}f
    printf("\033[%dD",x);
    cur_x-=x;
    tcdrain(1);
}
void z_terminal::curRight(uint x)
{
    //<ESC>[{ROW};{COLUMN}f
    printf("\x01b[%dC",x);
    cur_x+=x;
    //tcdrain(1);
}

/*
void z_terminal::SetUnderscore()
{
	//<ESC>[ {Ps} ; {Ps} m
	zout.putf("\x01b[0m");
}
*/

#endif



/*________________________________________________________________________

z_console_base
________________________________________________________________________*/

z_console_base::z_console_base()
{
    index = 0;
    cur_start = 0;
    len = 0;
    insertmode = true;
    _tab_mode = false;
}
uint z_console_base::get_index() { return index; }
uint z_console_base::get_line_length() { return len; }
void z_console_base::AppendChar(char ch)
{

    _buffer += ch;
    zout << ch;
    len++;
    cur_x++;
    index++;
}
void z_console_base::RedrawLine(int blanks)
{
    const char* s;
    s = _buffer;
    s += index;
    zout << s;
    while (blanks--) zout << ' ';
    curGotoXY(cur_x, cur_y);
}
void z_console_base::InsertChar(char ch)
{
    cur_x++;
    _buffer.insert(index, 1, ch);
    RedrawLine();
    len++;
    index++;
}
void z_console_base::OverwriteChar(char ch)
{
    cur_x++;
    _buffer.replace(index, 1, 1, ch);
    RedrawLine();
    index++;
}
void z_console_base::hChar(char ch)
{
    if (len == index)
    {
        AppendChar(ch);
        return;
    }
    if (insertmode)  InsertChar(ch);
    else OverwriteChar(ch);
}
void z_console_base::reset_line()
{
    index = 0;
    len = 0;
    _buffer = "";
    GetXY();
    cur_start = cur_x;

}
void z_console_base::output(ctext text)
{
    uint l = (uint)strlen(text);
    zout << text;
    len += l;
    _buffer += text;
    index += l;
    cur_x += l;
}
void z_console_base::clear_line()
{
    curLeft(index);
    while (len--) zout << ' ';
    curGotoXY(cur_x, cur_y);
    index = 0;
    len = 0;
    _buffer = "";
}
void z_console_base::trim_line_to(uint trim_point)
{
    uint amount_to_trim = len - trim_point;
    cur_x = cur_start + trim_point;
    curGotoXY(cur_x, cur_y);
    while (amount_to_trim--) zout << ' ';
    curGotoXY(cur_x, cur_y);
    index = trim_point;
    _buffer.erase(trim_point);
    //del(_buffer,trim_point);
    len = trim_point;
}

void z_console_base::quit()
{

    _console_running = false;
}

z_status z_console_base::run()
{

    terminal_open();
    _history_index = (uint)_history.size();
    _console_running = true;
    reset_line();
    put_prompt();

    _key = key_alpha;
    char ch;
    while (_console_running)
    {

        _prev_key = _key;
        GetKey(_key, ch);

        switch (_key)
        {
            case key_ctrl_C:
                _console_running = false;
                break;
            case key_insert:
                insertmode = !insertmode;
                break;
            case key_enter:
                OnEnter();
                break;
            case key_tab:
                if (_prev_key != key_tab) _tab_mode = false;
                OnTab();
                break;
            case key_up:

                OnUp();
                break;
            case key_down:

                OnDown();
                break;
            case key_end:
                if (index<len)
                {
                    curRight(len-index);
                    index = len;
                }
                break;
            case key_right:
                if (index<len)
                {
                    curRight(1);
                    index++;
                }
                break;
            case key_left:
                if (index>0)
                {
                    curLeft(1);
                    index--;
                }
                break;
            case key_home:
                if (index>0)
                {
                    curLeft(index);
                    index = 0;
                }

                break;
            case key_back:
                if (index>0)
                {
                    curLeft(1);
                    //zout << char_back;
                    index--;
                    len--;
                    _buffer.erase(index, 1);
                    //del(_buffer,index,1);
                    RedrawLine(1);
                }
                else
                {
                    if (_prev_key == key_back)
                    {
                        OnDoubleBack();
                    }
                }
                break;
            case key_delete:
                if (index<len)
                {

                    len--;
                    _buffer.erase(index, 1);
                    //del(_buffer,index,1);
                    RedrawLine(1);
                }
                break;
            case key_alpha:

                hChar(ch);
                zout.flush();
                break;
            default:
                break;
        }
    }
    return zs_ok;
}



/*
z_status z_console_base::parse_line(ctext text)
{
z_status status=_parser.parse_obj(&_cmdline,text);
if(status==	zs_ok)
{
return zs_ok;
}
else
_parser.report_error();
return zs_unknown_error;
}
*/
z_status z_console_base::ExecuteLine(ctext text)
{
    /*
    z_status status=parse_line(text);
    if(status)
    return status;
    */
    z_status status = zs_ok;


    return status;
}
void z_console_base::OnEnter()
{
    uint i;
    z_status result;
    zout << '\n';
    if (_buffer.size())
    {
        i = _history.find(_buffer);
        if (i != -1)
        {
            _history.del(i);
        }
        _history << _buffer;
        _history_index = (uint)_history.size();
    }
    if (_buffer.size())
    {
        //parse_line(_buffer,_zc);
        LogInput(_buffer);
        result = ExecuteLine(_buffer);
        /*
        Let the subclass handle errors
        if (result)
        {
            switch (result)
            {
            case zs_success:
            case zs_unknown_error:
                break;
            default:
                break;
            }
            Z_ERROR_MSG(result, "command failed: \"%s\"", zs_get_status_text(result));

        }
        */
        zout << '\n';

    }
    put_prompt();

}

void z_console_base::inc_history(uint i)
{
    uint history_count = _history.size();
    if (history_count == 0) return;
    clear_line();
    _history_index += i;
    if (_history_index >= history_count)
    {
        _history_index = 0;
    }
    if (_history_index<0)
    {
        _history_index = history_count - 1;
    }
    output(_history[_history_index]);
}
void z_console_base::OnUp()
{
    inc_history(-1);
};
void z_console_base::OnDown()
{
    inc_history(1);
};
void z_console_base::OnTab()
{
    zout << "\nTab.\n";
}

void z_console_base::OnDoubleBack()
{

    zout << "\n";
    put_prompt();
}
