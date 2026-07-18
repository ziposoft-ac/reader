
#include "zipolib/zipolib.h"
#include "zipolib/z_factory.h"
#include "zipolib/z_time.h"
#include "zipolib/z_console.h"
#include "zipolib/z_error.h"

#include <stdio.h>

class Toy {
    friend z_factory_t<Toy>;
public:
    z_status squeak() {
        std::cout << _name <<" goes squeak\n";
        return zs_ok;
    }
    z_string _name="name";
};

ZMETA(Toy) {
    ZACT(squeak);
    ZPROP(_name);
};

Toy gToy;

class Dog {
    friend z_factory_t<Dog>;
public:
    Toy woobie;
};
ZMETA(Dog) {
    ZOBJ_EX(gToy,"toy",ZFF_PROP_DEF,"squeaky ball");
    ZOBJ(woobie);


};

z_console console;
Dog gDog;
void ctrl_C_handler(int s) {
    console.quit();
    zout << "ctrl C handler\n";
};
int main(int argc, char* argv[]){
    z_catch_ctl_c(ctrl_C_handler);

    ZDBG("Hello World!");
    console.initialize(&gDog, argv[0]);

    //z_status status = console.loadcfg();

    console.runapp(argc, argv, true, 0);

    return 0;

}