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
    ZOBJ(cfmu804);
    ZOBJ(timerService);
    //ZOBJ(processRunner);
    ZOBJ(web_server);
    //ZOBJ(server);
    ZOBJ(app);
    ZOBJ(app0);
    ZACT(heat_test);
    ZACT(dump_ports);
    ZACT(run_app);
    ZPROP(_auto_start_server);
    ZPROP(_auto_start_app);
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
    app.initialize();
    gpio.initialize();
    if(_auto_start_server) {
        web_server.start();
        app.open();

    }
    if(_auto_start_app) {
        web_server.start();
        app.run();

    }
    return zs_ok;
}
z_status Root::shutdown()
{

    _cv_quit.notify_all();
    timerService.stop();
    web_server.stop();

    app.shutdown();
    cfmu804.close();+

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

int Root::heat_test_callback(void*)
{
    if(_shutting_down)
        return 0;
    _test_count++;

    cfmu804.stop();
    int count=cfmu804.getReadIndex();
    int queue=cfmu804.get_queue_depth();
    int temp=cfmu804.get_temperature_cmd();
    z_string ts=z_time::getTimeStrLocal();
    ZLOG("%s : ");
    printf("COUNT: %d QUEUE: %d TEMP:%d\n",count,queue,temp);
    if(temp>55) {
        printf("TEMP EXCESS! STOPPING TEST");

        return 0;

    }
    if(_shutting_down)
        return 0;
    cfmu804.start();
    return 60000;
}
z_status Root::heat_test()
{
    cfmu804.configure(rfid_config_heattest);

    cfmu804.readmode_get();
    cfmu804.info_dump();
    cfmu804.start();
    timerService.create_timer_t(this,&Root::heat_test_callback,0,2000);
    run_as_service();
    return zs_ok;
}
