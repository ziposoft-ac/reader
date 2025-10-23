#include "pch.h"

#include "root.h"

#include <filesystem>

void ctrl_C_handler(int s) {
    root.console.quit();
    root.quit_notify();
    zout << "ctrl C handler\n";
};
z_string get_now_time()
{
    return z_time::get_now_local().to_readable_string();
}
extern const  char* timestamp;

int eval(int val)
{
    printf("eval:%d\n",val);
    return val;
}


void testfunc(int first, int second,int third)
{
    printf("first:%d second:%d third:%d\n",first,second,third);

}
int isCwdWritable()
{
   FILE* f=fopen("dummy","w");
   if(f)
   {
       fclose(f);
       std::filesystem::remove("dummy");
       return true;
   }
   return false;

}

int main(int argc, char* argv[])
{

    printf("/a");


    srand(time(NULL));
    //testfunc(eval(1),eval(2),eval(3));
    //return 0;
    tzset();
    if(!isCwdWritable())
    {
        int res=chdir(getenv("HOME"));
        zout<< "Setting CWD to home="<<res<<"\n";
    }
    printf("\n========Zipo Timer=========\nBUILD: %s\n",timestamp);

    //z_filesys_setcwd();
    z_debug_load_save_args(&argc, &argv);
    z_string logname = argv[0];
    logname += ".log";
    get_zlog().fileout(logname);
#ifndef ARM
    //get_zlog().add_stdout();
#endif

    ZLOG("\n========Zipo Timer=========\n%s\n%s\nBUILD: %s\n",
         z_time::getTimeStrLocal().c_str(),
         z_time::getTimeStrGmt().c_str(),
         timestamp);
    ZTF;

    z_catch_ctl_c(ctrl_C_handler);
    get_zlog().get_stream_err().add_stdout();
    root.console.initialize(&root, argv[0]);

    z_status status = root.console.loadcfg();
    root.initialize();

    root.console.runapp(argc, argv, true, 0);
    root.shutdown();
    zout << "Ztimer2 EXIT\n";
}
