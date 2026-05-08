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
    ZACT(simulate_on);
    ZACT(simulate_off);
    ZACT(dump_ports);
    ZACT(run_app);
    ZPROP(_auto_start_server);
    ZPROP(_auto_start_app);
    ZPROP(_init_rfid);
    ZACT_X(run_as_service,"service", ZFF_ACT_DEF,"Run as service");
    ZPROP(_simulate);

};

Root::Root()  {
    _reader=&cfmu804;
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
    if (_simulate)
        _reader=&simulator;
    else
        _reader=&cfmu804;
    app0.initialize();
    gpio.initialize();
    if(_auto_start_server) {
        web_server.start();
        app0.open();

    }
    if(_auto_start_app) {
        web_server.start();
        app0.run();

    }
    if(_init_rfid) {
        _reader->open();
    }
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

