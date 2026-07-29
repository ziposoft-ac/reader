//
// Created by ac on 7/17/26.
//

#ifndef ZIPOSOFT_SERVICE_H
#define ZIPOSOFT_SERVICE_H
#include "pch.h"
#include "global.h"
#include "zipolib/lockfile.h"

#include "api/MqServer.h"
class Service;






class Service {
    friend z_factory_t<Service>;

    LockFile _lock_file;

    z_file_out _log_file;

    int _log_level=z_log_level_info;
    int _debug_level=z_log_level_debug;
public:
    Service() {
    }
    virtual ~Service() {}
    ctext getName() {
        return get_factory_from_vobj((z_void_obj*)this)->get_name();

    }
    bool globalLock() {
        z_string name;
        name.format("/tmp/service_%s.lock",getName());
        bool locked= _lock_file.lock(name);
        if (!locked) {
            Z_ERROR_LOG("Service is already locked\n");
            return false;
        }
        return true;
    }
    friend z_factory_t<Service>;
    virtual z_status shutdown();
    virtual z_status service();
    virtual z_status initialize();
    virtual z_status run();
    z_status init_logfile();

    virtual z_status remote_quit() {
        process_quit_notify();
        return zs_ok;

    }
    z_status debugLevelSet(int level) {
        get_debug_logger()._level=level;
        _debug_level=level;
        return zs_ok;
    }

};


template <class SERVICE> Service* getRootServiceT(z_factory** pfactory) {
    static SERVICE service;
    *pfactory = GET_FACT(SERVICE);
    return &service;


}

Service* getRootService(z_factory** factory);

#define ROOT_SERVICE(_TYPE_) _TYPE_ g##_TYPE_; Service* getRootService(z_factory** factory) { *factory=GET_FACT(_TYPE_);return &g##_TYPE_; }



ZMETA_DECL(Service) {
    ZCMD(debugLevelSet , ZFF_CMD_DEF, "debugLevelSet",
     ZPRM(int, level, 4, "level", ZFF_PARAM)
    );
    ZPROP(_log_level);
    ZPROP(_debug_level);
    ZACT(initialize);
    ZACT(shutdown);
    ZOBJ_EX(gConsole,"console",ZFF_PROP_DEF,"Console");
}

#endif //ZIPOSOFT_SERVICE_H
