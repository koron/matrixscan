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

static void hid_kb_report(uint8_t code, bool on);

void matrix_chagned(uint ncol, uint nrow, bool on, uint8_t when) {
    //printf("matrix_changed: col=%d row=%d %s when=%d\n", ncol, nrow, on ? "ON" : "OFF", when);
    uint8_t code = keymap[ncol + nrow * COLS_NUM];
    if (code != 0) {
        hid_kb_report(code, on);
    }
}

static void matrix_suppressed(uint ncol, uint nrow, bool on, uint8_t when, uint8_t last, uint8_t elapsed) {
    //printf("matrix_suppressed: col=%d row=%d %s when=%d last=%d elapsed=%d\n", ncol, nrow, on ? "ON" : "OFF", when, last, elapsed);
}

// matrix_task scan whole switch matrix.
void matrix_task() {
    static uint8_t count = 0;
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

static bool hid_kb_changed = false;
static uint8_t hid_kb_mod = 0;
static uint8_t hid_kb_code[6] = {0};

// hid_kb_report composes keyboard report which will be send.
static void hid_kb_report(uint8_t code, bool on) {
    //printf("hid_kb_report: %02X %s\n", code, on ? "ON" : "OFF");

    // consider modifier.
    uint8_t mod = 0;
    switch (code) {
        case HID_KEY_CONTROL_LEFT:
            mod = KEYBOARD_MODIFIER_LEFTCTRL;
            break;
        case HID_KEY_SHIFT_LEFT:
            mod = KEYBOARD_MODIFIER_LEFTSHIFT;
            break;
        case HID_KEY_ALT_LEFT:
            mod = KEYBOARD_MODIFIER_LEFTALT;
            break;
        case HID_KEY_GUI_LEFT:
            mod = KEYBOARD_MODIFIER_LEFTGUI;
            break;
        case HID_KEY_CONTROL_RIGHT:
            mod = KEYBOARD_MODIFIER_RIGHTCTRL;
            break;
        case HID_KEY_SHIFT_RIGHT:
            mod = KEYBOARD_MODIFIER_RIGHTSHIFT;
            break;
        case HID_KEY_ALT_RIGHT:
            mod = KEYBOARD_MODIFIER_RIGHTALT;
            break;
        case HID_KEY_GUI_RIGHT:
            mod = KEYBOARD_MODIFIER_RIGHTGUI;
            break;
    }
    if (mod != 0) {
        if (on) {
            hid_kb_mod |= mod;
        } else {
            hid_kb_mod &= ~mod;
        }
        hid_kb_changed |= true;
        return;
    }

    // change hid_kb_code.
    int found = -1, vacant = -1;
    for (int i = 0; i < COUNT_OF(hid_kb_code); i++) {
        if (vacant < 0 && hid_kb_code[i] == 0) {
            vacant = i;
        }
        if (found < 0 && hid_kb_code[i] == code) {
            found = i;
        }
    }
    // when key up.
    if (!on) {
        if (found >= 0) {
            hid_kb_code[found] = 0;
            hid_kb_changed |= true;
        }
        return;
    }
    // when key down.
    if (found >= 0 || vacant < 0) {
        return;
    }
    hid_kb_code[vacant] = code;
    hid_kb_changed |= true;
}

void hid_task() {
    if (hid_kb_changed) {
        // clean up hid_kb_code. remove intermediate zeros, padding non-zero
        // codes to left.
        bool aligned = false;
        uint8_t tmp[6] = {0};
        for (int i = 0, j = 0; i < COUNT_OF(hid_kb_code); i++) {
            if (hid_kb_code[i] != 0) {
                tmp[j] = hid_kb_code[i];
                if (j != i) {
                    aligned |= true;
                }
                j++;
            }
        }
        if (aligned) {
            memcpy(hid_kb_code, tmp, sizeof(hid_kb_code));
        }
        // send keyboard report with boot protocol.
        //printf("hid_task: keyboard: %02X (%02X %02X %02X %02X %02X %02X)\n", hid_kb_mod, hid_kb_code[0], hid_kb_code[1], hid_kb_code[2], hid_kb_code[3], hid_kb_code[4], hid_kb_code[5]);
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, hid_kb_mod, hid_kb_code);
        // clear changed flag.
        hid_kb_changed = false;
    }
}
