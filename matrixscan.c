#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "tusb.h"

#include "config.h"
#include "usb_descriptors.h"

static const uint matrix_cols[] = MATRIX_COLS;
static const uint matrix_rows[] = MATRIX_ROWS;

// scan_interval is interval or each matrix scan.
static const uint16_t scan_interval = 30;
// debounce is inhibition internal for changing status of each keys.
static const uint16_t debounce = 5;

#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))
#define COLS_NUM COUNT_OF(matrix_cols)
#define ROWS_NUM COUNT_OF(matrix_rows)

static uint32_t mask_cols[COLS_NUM];
static uint32_t mask_rows[ROWS_NUM];
static uint32_t row_mask;

typedef struct {
    bool on;
    uint8_t last;
} scan_state;

static scan_state matrix_states[COLS_NUM * ROWS_NUM];

uint8_t keymap[COLS_NUM * ROWS_NUM] = {
    HID_KEY_ESCAPE, HID_KEY_TAB, HID_KEY_1, HID_KEY_2, HID_KEY_3, HID_KEY_4, HID_KEY_5, HID_KEY_MINUS, HID_KEY_BACKSLASH, HID_KEY_BACKSPACE,
    HID_KEY_Q, HID_KEY_W, HID_KEY_E, HID_KEY_R, HID_KEY_T, HID_KEY_Y, HID_KEY_U, HID_KEY_I, HID_KEY_O, HID_KEY_P,
    HID_KEY_A, HID_KEY_S, HID_KEY_D, HID_KEY_F, HID_KEY_G, HID_KEY_H, HID_KEY_J, HID_KEY_K, HID_KEY_L, HID_KEY_SEMICOLON,
    HID_KEY_Z, HID_KEY_X, HID_KEY_C, HID_KEY_V, HID_KEY_B, HID_KEY_N, HID_KEY_M, HID_KEY_COMMA, HID_KEY_PERIOD, HID_KEY_SLASH,
    0, 0, HID_KEY_ALT_LEFT, 0, HID_KEY_SPACE, HID_KEY_GUI_RIGHT, HID_KEY_RETURN, HID_KEY_SHIFT_RIGHT, HID_KEY_CONTROL_RIGHT, 0
};

void matrix_chagned(uint ncol, uint nrow, bool on, uint8_t when) {
    printf("matrix: changed col=%d row=%d %s when=%d\n",
            ncol, nrow, on ? "ON" : "OFF", when);
    uint x = ncol + nrow * COLS_NUM;
    uint8_t kc = keymap[x];
    if (on && kc != 0) {
        uint8_t keycode[6] = {0};
        keycode[0] = kc;
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        return;
    }
    if (!on) {
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        return;
    }
}

void matrix_chagne_suppressed(uint ncol, uint nrow, bool on, uint8_t when, uint8_t last, uint8_t elapsed) {
    //printf("matrix: suppressed: col=%d row=%d %s when=%d last=%d elapsed=%d\n", ncol, nrow, on ? "ON" : "OFF", when, last, elapsed);
}

bool matrix_task() {
    static uint8_t count = 0;
    bool changed = false;
    uint x = 0;
    for (uint nrow = 0; nrow < ROWS_NUM; nrow++) {
        gpio_put_masked(row_mask, mask_rows[nrow]);
        sleep_us(scan_interval);
        uint32_t bits = gpio_get_all();
        for (uint ncol = 0; ncol < COLS_NUM; ncol++) {
            bool on = (bits & mask_cols[ncol]) == 0;
            if (on != matrix_states[x].on) {
                uint8_t elapsed = count - matrix_states[x].last;
                if (elapsed >= debounce) {
                    matrix_states[x].on = on;
                    matrix_states[x].last = count;
                    matrix_chagned(ncol, nrow, on, count);
                    changed |= true;
                } else {
                    matrix_chagne_suppressed(ncol, nrow, on, count, matrix_states[x].last, elapsed);
                }
            }
            x++;
        }
    }
    count++;
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
