/*
Core zipolib functions

*/

#if 0
#include "pch.h"
#include "zipolib/zipolib.h"
#include <inttypes.h>
#include <signal.h>
#include "zipolib/z_strlist.h"
#include "zipolib/z_file.h"

#if 0
int MyAllocHook(int allocType, void* userData, size_t size,
	int blockType, long requestNumber,
	const unsigned char* filename, int lineNumber)
{
	if (size == 888)
	{
		int i = 0;

	}

	return TRUE;

}
#endif

int WindowsReportHook(int reportType, char* message, int* returnValue)
{

	return(1);

}

zipo_library g_zipo_library;
zipo_library::zipo_library()
{
	_argv = 0;
#ifdef BUILD_VSTUDIO
#ifdef _DEBUG

	//TODO combine this with 
	// init mem debug stuff
	_debug_log_file = (size_t)CreateFile("memeoryleaks.txt", // open Two.txt
		FILE_APPEND_DATA,         // open for writing
		FILE_SHARE_WRITE,          // allow multiple readers
		NULL,                     // no security
		OPEN_ALWAYS,              // open or create
		FILE_ATTRIBUTE_NORMAL,    // normal file
		NULL);                    // no attr. template

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
	//_CrtSetReportMode(_CRT_WARN,  _CRTDBG_MODE_DEBUG);
	_CrtSetReportFile(_CRT_WARN, (HANDLE)_debug_log_file);
	//_CrtSetAllocHook(MyAllocHook);
	//_CrtSetReportHook(WindowsReportHook);


#endif
#endif

	//std::ios_base::sync_with_stdio(false);
	// init debeg log


}

zipo_library::~zipo_library()
{
#ifdef BUILD_VSTUDIO
#ifdef _DEBUG
	//_CrtDumpMemoryLeaks();
#endif
#endif
	if (_argv)
		delete _argv;
}




bool  zipo_library::debug_load_save_args(int* pargc, char*** pargv)
{
	int argc = *pargc;
	char** argv = *pargv;
	int i;
#ifdef DEBUG
	for (i = 0; i < argc; i++)
	{
		ZT("ARG %d=%s \n", i, argv[i]);
	}
#endif
	if ((argc == 2) && (strcmp(argv[1], "debug") == 0))
	{

		z_FileReadAllLines("debug.txt", _arg_list);
		//check if we have saved args
		if (!_arg_list.size())
			return false; //no saved args, abort
		_argc = _arg_list[0].get_int_val();
		//check that saved argc matches list length
		if (_arg_list.size() < (size_t)(_argc + 1))
			return false;//bad list, abort
		_argv = (char**)z_new char*[_argc];

		//preserve the exe name. if last run was some other process
		_argv[0] = argv[0]; 
		for (i = 1; i < _argc; i++)
		{
			_argv[i] = (char*) (_arg_list[i + 1].c_str());

		}
		// replace main() args with our saved args
		*pargc = _argc;
		*pargv = _argv;
		return true;
	}
	else
	{
		std::ofstream  file("debug.txt");
		if (file.is_open())
		{
			file << argc << '\n';
			int i;
			for (i = 0; i < argc; i++)
				file << argv[i] << '\n';
		}
	}
	return false;
}

bool  z_debug_load_save_args(int* pargc, char*** pargv)
{
	return g_zipo_library.debug_load_save_args(pargc, pargv);
}





// This contains all the globals to be initialized for the library
#ifdef WINDOWS

void sleep(int sec)
{
	Sleep(1000 * sec);
}






z_catch_ctl_c_handler_t g_catch_ctl_c_handler = 0;
BOOL WINAPI  WindowsCtrlHandler(DWORD fdwCtrlType)
{
	ZTF;
	g_catch_ctl_c_handler(fdwCtrlType);
	switch (fdwCtrlType)
	{
		// Handle the CTRL-C signal. 
	case CTRL_C_EVENT:
		return(TRUE);

		// CTRL-CLOSE: confirm that the user wants to exit. 
	case CTRL_CLOSE_EVENT:
		return(TRUE);

		// Pass other signals to the next handler. 
	case CTRL_BREAK_EVENT:
		//Beep(900, 200);
		printf("Ctrl-Break event\n\n");
		return FALSE;

	case CTRL_LOGOFF_EVENT:
		return FALSE;

	case CTRL_SHUTDOWN_EVENT:
		return FALSE;

	default:
		return FALSE;
	}
	return TRUE;
}
#endif
z_status z_catch_ctl_c(z_catch_ctl_c_handler_t handler)
{
#ifdef WINDOWS
	ZTF;
	if (SetConsoleCtrlHandler(WindowsCtrlHandler, TRUE))
	{
		g_catch_ctl_c_handler = handler;
		return zs_ok;
	}
	else
	{
		printf("\nERROR: Could not set control handler\n");
		return zs_internal_error;
	}
#else
	struct sigaction sigIntHandler;

	sigIntHandler.sa_handler = handler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;

	sigaction(SIGTERM, &sigIntHandler, NULL);
	sigaction(SIGINT, &sigIntHandler, NULL);
	sigaction(SIGPIPE, &sigIntHandler, NULL);
#endif
	return zs_ok;


}



#endif