#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

uint matrix_cols[] = { 8, 9, 10, 11, 12, 13, 14, 15, 16, 17 };
uint matrix_rows[] = { 18, 19, 20, 21, 22 };

#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))

void matrix_init(uint* ports, int num, bool out) {
    for (int i = 0; i < num; i++) {
        uint io = ports[i];
        printf("setup GPIO=%d %s\n", io, out ? "OUT" : "IN");
        gpio_init(io);
        gpio_set_dir(io, out);
        gpio_pull_up(io);
    }
}

int main()
{
    stdio_init_all();
    printf("Matrix Scanner start\n");

    const int cols_num = COUNT_OF(matrix_cols);
    const int rows_num = COUNT_OF(matrix_rows);

    matrix_init(matrix_cols, cols_num, GPIO_IN);
    matrix_init(matrix_rows, rows_num, GPIO_OUT);

    while (true) {
        for (int nrow = 0; nrow < rows_num; nrow++) {
            uint io_row = matrix_rows[nrow];
            gpio_pull_down(io_row);
            sleep_us(30);
            for (int ncol = 0; ncol < cols_num; ncol++) {
                uint io_col = matrix_cols[ncol];
                bool hi = gpio_get(io_col);
                if (!hi) {
                    printf("col=%d row=%d: ON\n", ncol, nrow);
                }
            }
            gpio_pull_up(io_row);
        }
    }

    return 0;
}
