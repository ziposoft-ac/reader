

#include "zipolib/z_error.h"

#include <stdio.h>

int main() {

    ZDBG("Hello World!");
    return Z_ERROR(zs_access_denied);

}