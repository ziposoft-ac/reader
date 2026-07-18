#include "pch.h"
#include "global.h"


std::condition_variable g_process_quit_cv;
std::mutex g_process_quit_mutex;
bool g_process_shutting_down = false;

z_console console;


typedef int (*ShutdownCallback)(void* data);



void process_wait_for_quit()
{
    if (console.is_console_running())
        return; //

    std::unique_lock<std::mutex> mlock(g_process_quit_mutex);

    g_process_quit_cv.wait(mlock);
}
void process_quit_notify()
{
    printf("root quit_notify\n");
    g_process_shutting_down=true;
    g_process_quit_cv.notify_all();
    console.quit();

}
void ctrl_C_handler(int s) {
    console.quit();
    process_quit_notify();
    zout << "ctrl C handler\n";
};


int main(int argc, char* argv[])
{
    srand(time(NULL));
    tzset();

    if (!gRootObject) {
        Z_ERROR_LOG("No root object defined\n");
        return -1;
    }
    z_catch_ctl_c(ctrl_C_handler);
    z_debug_load_save_args(&argc, &argv);

    console.initialize_vobj(gRootObject, argv[0]);

#ifdef UGLY_CRAP
    z_factory* factory =   get_factory_from_vobj(gRootObject);
    zf_action* init_act=factory->get_action("initialize");
    if (init_act) {
        zf_command_context cc(&console);
        init_act->execute(cc);
    }
#endif


    console.runapp(argc, argv, true, 0);



}
