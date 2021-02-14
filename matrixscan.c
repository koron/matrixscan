#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "config.h"

static const uint col_pins[] = COL_PINS;
static const uint row_pins[] = ROW_PINS;

// scan_interval is interval for each row scanning of matrix (us).
static const uint16_t scan_interval = 30;
// debounce is inhibition interval for changing status of each keys.
static const uint16_t debounce = 5;

static uint32_t row_scanmask;
static uint32_t row_scanval[ROW_NUM];
static uint32_t col_masks[COL_NUM];

typedef struct {
    bool on;
    uint8_t last;
} scan_state;

static scan_state matrix_states[COL_NUM * ROW_NUM];

#if 0
static void matrix_changed(uint ncol, uint nrow, bool on, uint8_t when) {
    //printf("matrix_changed: col=%d row=%d %s when=%d\n", ncol, nrow, on ? "ON" : "OFF", when);
    uint8_t code = layer_getcode(ncol, nrow, on);
    if (code != 0) {
        hidkb_report_code(code, on);
    }
}

static void matrix_suppressed(uint ncol, uint nrow, bool on, uint8_t when, uint8_t last, uint8_t elapsed) {
    //printf("matrix_suppressed: col=%d row=%d %s when=%d last=%d elapsed=%d\n", ncol, nrow, on ? "ON" : "OFF", when, last, elapsed);
}
#endif

__attribute__((weak)) void matrix_changed(uint ncol, uint nrow, bool on, uint8_t when) {
    printf("matrix_changed: col=%d row=%d %s when=%d\n", ncol, nrow, on ? "ON" : "OFF", when);
}

__attribute__((weak)) void matrix_suppressed(uint ncol, uint nrow, bool on, uint8_t when, uint8_t last, uint8_t elapsed) {
    printf("matrix_suppressed: col=%d row=%d %s when=%d last=%d elapsed=%d\n", ncol, nrow, on ? "ON" : "OFF", when, last, elapsed);
}

// matrix_task scan whole switch matrix.
void matrix_task() {
    static uint8_t count = 0;
    uint x = 0;
    for (uint nrow = 0; nrow < ROW_NUM; nrow++) {
        gpio_put_masked(row_scanmask, row_scanval[nrow]);
        sleep_us(scan_interval);
        uint32_t bits = gpio_get_all();
        for (uint ncol = 0; ncol < COL_NUM; ncol++) {
            bool on = (bits & col_masks[ncol]) == 0;
            if (on != matrix_states[x].on) {
                uint8_t elapsed = count - matrix_states[x].last;
                if (elapsed >= debounce) {
                    matrix_states[x].on = on;
                    matrix_states[x].last = count;
                    matrix_changed(ncol, nrow, on, count);
                } else {
                    matrix_suppressed(ncol, nrow, on, count, matrix_states[x].last, elapsed);
                }
            }
            x++;
        }
    }
    count++;
}

void matrix_init() {
    //printf("matrix_init: sizeof(matrix_states)=%d\n", sizeof(matrix_states));
    // setup pins of columns.
    for (int i = 0; i < COL_NUM; i++) {
        uint io = col_pins[i];
        gpio_init(io);
        gpio_set_dir(io, GPIO_IN);
        gpio_pull_up(io);
        col_masks[i] = 1ul << io;
    }
    // setup pins of rows.
    uint32_t mask = 0;
    for (int i = 0; i < ROW_NUM; i++) {
        uint io = row_pins[i];
        gpio_init(io);
        gpio_set_dir(io, GPIO_OUT);
        row_scanval[i] = ~(1ul << io);
        mask |= 1ul << io;
    }
    // compose scan mask.
    row_scanmask = mask;
    for (int i = 0; i < ROW_NUM; i++) {
        row_scanval[i] &= mask;
    }
    memset(matrix_states, 0, sizeof(matrix_states));
}
