

#include "zipolib/z_error.h"

#include <stdio.h>

int main() {

    printf("tests/test_logging/test_logging.c(10)\n");
    return Z_ERROR(zs_access_denied);

}