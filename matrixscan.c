#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "config.h"

static const uint matrix_cols[] = MATRIX_COLS;
static const uint matrix_rows[] = MATRIX_ROWS;

// scan_interval is interval or each matrix scan.
static const uint16_t scan_interval = 30;
// debounce is inhibition internal for changing status of each keys.
static const uint16_t debounce = 150;

#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))
#define COLS_NUM COUNT_OF(matrix_cols)
#define ROWS_NUM COUNT_OF(matrix_rows)

static uint32_t mask_cols[COLS_NUM];
static uint32_t mask_rows[ROWS_NUM];
static uint32_t row_mask;

typedef struct {
    bool on:1;
    uint16_t ts:15;
} scan_state;

static scan_state matrix_states[COLS_NUM * ROWS_NUM];

void matrix_chagned(uint ncol, uint nrow, bool on) {
    // TODO: send HID code.
    printf("matrix: col=%d row=%d: %s\n", ncol, nrow, on ? "ON" : "OFF");
}

bool matrix_task() {
    bool changed = false;
    uint x = 0;
    uint16_t now = (uint16_t)(time_us_32()) & 0x7fff;
    for (uint nrow = 0; nrow < ROWS_NUM; nrow++) {
        gpio_put_masked(row_mask, mask_rows[nrow]);
        sleep_us(scan_interval);
        uint32_t bits = gpio_get_all();
        for (uint ncol = 0; ncol < COLS_NUM; ncol++) {
            bool on = (bits & mask_cols[ncol]) == 0;
            if (on != matrix_states[x].on && now - matrix_states[x].ts >= debounce) {
                matrix_states[x].on = on;
                matrix_states[x].ts = now;
                matrix_chagned(ncol, nrow, on);
                changed |= true;
            }
            x++;
        }
    }
}

void matrix_init() {
    printf("matrix: sizeof(matrix_states)=%d\n", sizeof(matrix_states));
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
    memset(matrix_states, 0, sizeof(matrix_states));
}
