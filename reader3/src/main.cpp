#include "pch.h"

#include "root.h"


#include "zipolib/z_log.h"
#include "zipolib/lockfile.h"

void ctrl_C_handler(int s) {
    root.console.quit();
    root.quit_notify();
    zout << "ctrl C handler\n";
};
z_string get_now_time()
{
    return z_time::get_now_local().to_readable_string();
}

int eval(int val)
{
    printf("eval:%d\n",val);
    return val;
}


void testfunc(int first, int second,int third)
{
    printf("first:%d second:%d third:%d\n",first,second,third);

}

int main(int argc, char* argv[])
{


    srand(time(NULL));
    //testfunc(eval(1),eval(2),eval(3));
    //return 0;
    tzset();



    z_debug_load_save_args(&argc, &argv);


#ifndef ARM
    //get_zlog().add_stdout();
#endif


    z_catch_ctl_c(ctrl_C_handler);
    root.console.initialize(&root, argv[0]);

    z_status status = root.console.loadcfg();
    root.initialize();

    root.console.runapp(argc, argv, true, 0);
    root.shutdown();

}
