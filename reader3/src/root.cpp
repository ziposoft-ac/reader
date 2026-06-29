//
// Created by ac on 11/13/20.
//

#include "root.h"

Root root;

ZMETA(Root)
{

    ZOBJ(console);
    ZOBJ(simulator);
    ZOBJ(gpio);
    ZOBJ(tests);
    ZOBJ_X(cfmu804,"rfid",ZFF_PROP_DEF,"cf-804 module");
    ZOBJ(timerService);
    //ZOBJ(processRunner);
    ZOBJ(web_server);
    //ZOBJ(server);
    ZOBJ(app0);
    ZACT(dump_ports);
    ZACT(run_app);
    ZACT_X(run_as_service,"service", ZFF_ACT_DEF,"Run as service");

};

Root::Root()  {
}

Root::~Root() {

}

z_status Root::dump_ports()
{
    std::vector<serial::PortInfo> list=serial::list_ports();
    for (auto i : list)
    {
        zout << i.port << " " << i.hardware_id << " " << i.description << "\n";
    }
    return zs_ok;
}
z_status Root::run_as_service()
{
    if (console.is_console_running())
        return zs_already_open;
    web_server.start();
    wait_for_quit();
    return zs_ok;
}
z_status Root::run_app()
{
    return zs_ok;
}
z_status Root::initialize()
{

    return zs_ok;
}
z_status Root::shutdown()
{

    _cv_quit.notify_all();
    timerService.stop();
    web_server.stop();

    app0.shutdown();
    cfmu804.close();

    simulator.close();
    gpio.shutdown();

    return zs_ok;
}

z_status Root::quit_notify()
{
    printf("root quit_notify\n");
    _shutting_down=true;
    _cv_quit.notify_all();
    console.quit();

    return zs_ok;
}

