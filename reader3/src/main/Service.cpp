//
// Created by ac on 7/17/26.
//

#include "Service.h"
#include <filesystem>

#include "global.h"

Service *gService = NULL;

z_status Service::service() {

    if (!gConsole.is_console_running()) {
        process_wait_for_quit();
        ZLOG("\n======== Service %s exit:%s =========\n",getName(), z_time::getTimeStrLocal().c_str() );

    }
    return zs_ok;
}

z_status Service::shutdown() {
    gTimerService.shutdown();

    return zs_ok;
}

z_status Service::initialize() {
    if (!globalLock()) {
        printf("Cannot acquire global lock, service already running\n");
        return zs_already_open;

    }
    gTimerService.init();
    return zs_ok;

}

z_status Service::run_as_service() {
    if (gConsole.is_console_running())
        return zs_already_open;
    process_wait_for_quit();
    return zs_ok;


}

z_status Service::init_logfile() {

    std::error_code ec;
    z_string logname=getName();
    logname+="_logs";
    std::filesystem::create_directory(logname.c_str());
    logname+="/";

    logname+=z_time::getTimeStrLocalFsFormat()+".log";
    _log_file.open(logname);
    get_default_logger().always_flush=true;
    get_default_logger().create_file_out(logname);
    std::filesystem::remove("last.log",ec);
    std::filesystem::create_symlink(logname.c_str(),"last.log",ec);
    z_string ts=z_time::getTimeStrLocal();

    ZLOG("\n========%s=========\nLOCAL:%s\nGMT:%s\nBUILD: %s\n",
        getName(),
         z_time::getTimeStrLocal().c_str(),
         z_time::getTimeStrGmt().c_str(),
         BUILD_TIME_STAMP);
    ZTF;

    return zs_ok;

}


