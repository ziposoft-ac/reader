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
    ZOBJ(readerService);
    ZOBJ(button);
    //ZOBJ(processRunner);
    ZOBJ(web_server);
    ZOBJ(beeper);
    //ZOBJ(server);
    ZOBJ(visitProc);
    ZACT(simulate_on);
    ZACT(simulate_off);
    ZACT(dump_ports);
    ZPROP_X(_enable_logging,"logging",ZFF_PROP_NOLOAD,"Enable logging to console");

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


z_status Root::initialize()
{
    if (_simulate)
        _reader=&simulator;
    else
        _reader=&cfmu804;
    gpio.initialize();


    return zs_ok;
}
z_status Root::shutdown()
{

    _cv_quit.notify_all();
    timerService.stop();
    web_server.stop();

    visitProc.shutdown();
    cfmu804.close();

    simulator.close();
    gpio.shutdown();
    beeper.shutdown();
    button.stop();

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

