#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "config.h"

static const uint matrix_cols[] = MATRIX_COLS;
static const uint matrix_rows[] = MATRIX_ROWS;

#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))
#define COLS_NUM COUNT_OF(matrix_cols)
#define ROWS_NUM COUNT_OF(matrix_rows)

static uint32_t mask_cols[COLS_NUM];
static uint32_t mask_rows[ROWS_NUM];
static uint32_t row_mask;

bool matrix_states[COLS_NUM * ROWS_NUM];

void matrix_chagned(uint ncol, uint nrow, bool on) {
    // TODO: send HID code.
    printf("col=%d row=%d: %s\n", ncol, nrow, on ? "ON" : "OFF");
}

void matrix_scan() {
    uint x = 0;
    for (uint nrow = 0; nrow < ROWS_NUM; nrow++) {
        gpio_put_masked(row_mask, mask_rows[nrow]);
        sleep_us(30);
        uint32_t bits = gpio_get_all();
        for (uint ncol = 0; ncol < COLS_NUM; ncol++) {
            bool on = (bits & mask_cols[ncol]) == 0;
            if (on != matrix_states[x]) {
                matrix_states[x] = on;
                matrix_chagned(ncol, nrow, on);
            }
            x++;
        }
    }
}

void matrix_init() {
    for (int i = 0; i < COLS_NUM; i++) {
        uint io = matrix_cols[i];
        gpio_init(io);
        gpio_set_dir(io, GPIO_IN);
        gpio_pull_up(io);
        mask_cols[i] = 1ul << io;
    }
    uint32_t mask = 0;
    for (int i = 0; i < ROWS_NUM; i++) {
        uint io = matrix_rows[i];
        gpio_init(io);
        gpio_set_dir(io, GPIO_OUT);
        mask_rows[i] = ~(1ul << io);
        mask |= 1ul << io;
    }
    row_mask = mask;
    for (int i = 0; i < ROWS_NUM; i++) {
        mask_rows[i] &= mask;
    }
    for (int i = 0; i < COUNT_OF(matrix_states); i++) {
        matrix_states[i] = false;
    }
}

int main()
{
    stdio_init_all();
    printf("Matrix Scanner start\n");

    matrix_init();

    while (true) {
        matrix_scan();
    }

    return 0;
}
