//
// Created by ac on 7/17/26.
//

#include "Service.h"
#include <filesystem>


Service *gService = NULL;

z_status Service::service() {

    if (!gConsole.is_console_running()) {
        process_wait_for_quit();
        ZLOG("\n======== Service %s exit:%s =========\n",getName(), z_time::getTimeStrLocal().c_str() );

    }
    return zs_ok;
}

z_status Service::shutdown() {
    return zs_ok;
}

z_status Service::initialize() {
    return zs_ok;

}

z_status Service::run() {
    return zs_ok;

}

z_status Service::init_logfile() {

    std::error_code ec;

    std::filesystem::create_directory("logs");
    z_string logname="logs/reader-";
    logname+=z_time::getTimeStrLocalFsFormat()+".log";
    _log_file.open(logname);
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


