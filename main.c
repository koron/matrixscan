#include <stdio.h>
#include "pico/stdlib.h"
#include "matrixscan.h"

int main()
{
    stdio_init_all();
    printf("Prototype Keyboard start\n");

    matrix_init();

    while (true) {
        matrix_scan();
    }

    return 0;
}
