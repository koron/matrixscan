#include "pico/stdlib.h"
#include "tusb.h"

#include "config.h"
#include "keycodes.h"

static uint8_t keymap[COL_NUM * ROW_NUM] = {
    KC_ESCAPE, KC_TAB, KC_1, KC_2, KC_3, KC_4, KC_5, KC_MINUS, KC_BSLASH, KC_BSPACE,
    KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P,
    KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCOLON,
    KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMMA, KC_DOT, KC_SLASH,
    0, 0, KC_LALT, 0, KC_SPACE, KC_RGUI, KC_RETURN, KC_RSHIFT, KC_RCTRL, 0
};

uint8_t layer_getcode(uint ncol, uint nrow, bool on) {
    return keymap[ncol + nrow * COL_NUM];
}
