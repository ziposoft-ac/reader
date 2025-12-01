#include "pch.h"

#include "root.h"

#include <filesystem>

#include "zipolib/z_log_old.h"

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

    std::error_code ec;

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
    auto path=std::filesystem::current_path( ec );

    //printf("path=%s\n",path.string().c_str());
    std::filesystem::create_directory("logs");

    z_string ts=z_time::getTimeStrLocal();

    //z_filesys_setcwd();
    z_debug_load_save_args(&argc, &argv);
    z_string logname =path.string();
    logname="logs/reader-";
    //logname+=argv[0];
    logname+=z_time::getTimeStrLocalFsFormat()+".log";
    z_file_out log_file(logname);
    get_default_logger().create_file_out(logname);
    std::filesystem::remove("last.log",ec);
    std::filesystem::create_symlink(logname.c_str(),"last.log",ec);

#ifndef ARM
    //get_zlog().add_stdout();
#endif

    ZLOG("\n========Zipo Timer=========\nLOCAL:%s\nGMT:%s\nBUILD: %s\n",
         z_time::getTimeStrLocal().c_str(),
         z_time::getTimeStrGmt().c_str(),
         timestamp);
    ZTF;

    z_catch_ctl_c(ctrl_C_handler);
    root.console.initialize(&root, argv[0]);

    z_status status = root.console.loadcfg();
    root.initialize();

    root.console.runapp(argc, argv, true, 0);
    root.shutdown();
    ZLOG("\n======== rfid reader exit LOCAL:%s =========\n",
     z_time::getTimeStrLocal().c_str()
     );
}
