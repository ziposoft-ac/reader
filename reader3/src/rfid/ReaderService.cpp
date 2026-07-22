//
// Created by ac on 6/29/26.
//
#include "../root.h"
#include "zipolib/lockfile.h"
#include <filesystem>

#include "ReaderService.h"


ZMETA_DEF(ReaderService);


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
extern const  char* BUILD_TIME_STAMP;


z_status ReaderService::run() {


    if (_running)
        return zs_already_open;

    LockFile lock_file;
    if (!lock_file.lock("/tmp/reader_service.lock")) {
        printf("\nCannot acquire lock file\n");
        return zs_already_open;

    }
    if(!isCwdWritable())
    {
        int res=chdir(getenv("HOME"));
        zout<< "Setting CWD to home="<<res<<"\n";
    }
    //auto path=std::filesystem::current_path( ec );
    //z_string logname =path.string();
    std::error_code ec;

    std::filesystem::create_directory("logs");
    z_string logname="logs/reader-";
    logname+=z_time::getTimeStrLocalFsFormat()+".log";
    z_file_out log_file(logname);
    get_default_logger().create_file_out(logname);
    std::filesystem::remove("last.log",ec);
    std::filesystem::create_symlink(logname.c_str(),"last.log",ec);
    z_string ts=z_time::getTimeStrLocal();

    ZLOG("\n========Zipo Timer=========\nLOCAL:%s\nGMT:%s\nBUILD: %s\n",
         z_time::getTimeStrLocal().c_str(),
         z_time::getTimeStrGmt().c_str(),
         BUILD_TIME_STAMP);
    ZTF;

    _running=true;
    root.web_server.start();
    if (!root.console.is_console_running()) {
        root.wait_for_quit();
        ZLOG("\n======== rfid reader exit LOCAL:%s =========\n", z_time::getTimeStrLocal().c_str() );

    }
    return zs_ok;


}
