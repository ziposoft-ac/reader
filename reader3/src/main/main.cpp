#include "pch.h"
#include "global.h"
#include "Service.h"


std::condition_variable g_process_quit_cv;
std::mutex g_process_quit_mutex;
bool g_process_shutting_down = false;

z_console gConsole;


typedef int (*ShutdownCallback)(void* data);



void process_wait_for_quit()
{
    if (gConsole.is_console_running())
        return; //

    std::unique_lock<std::mutex> mlock(g_process_quit_mutex);

    g_process_quit_cv.wait(mlock);
}
void process_quit_notify()
{
    printf("root quit_notify\n");
    g_process_shutting_down=true;
    g_process_quit_cv.notify_all();
    gConsole.quit();

}
void ctrl_C_handler(int s) {
    gConsole.quit();
    process_quit_notify();
    zout << "ctrl C handler\n";
};

extern const  char* BUILD_TIME_STAMP;

int main(int argc, char* argv[])
{
    ZDBG("main\n");
    srand(time(NULL));
    tzset();

    if (argc==1) {
        ZLOG("\n========Zipo=========\nLOCAL:%s\nGMT:%s\nBUILD: %s\n",
     z_time::getTimeStrLocal().c_str(),
     z_time::getTimeStrGmt().c_str(),
     BUILD_TIME_STAMP);

    }
    z_catch_ctl_c(ctrl_C_handler);
    z_debug_load_save_args(&argc, &argv);
    z_factory* factory=0;
    Service* service=getRootService(&factory);
    gConsole.initialize(service,factory, argv[0]);

#ifdef UGLY_CRAP
    z_factory* factory =   get_factory_from_vobj(gRootObject);
    zf_action* init_act=factory->get_action("initialize");
    if (init_act) {
        zf_command_context cc(&console);
        init_act->execute(cc);
    }
#endif
    service->initialize();

    gConsole.runapp(argc, argv, true, 0);

    service->shutdown();


}
