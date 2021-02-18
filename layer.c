#include "pico/stdlib.h"
#include "tusb.h"

#include "config.h"
#include "keycodes.h"

#define LAYER_MAXNUM 31

uint32_t layer_state = 0;

bool layer_is_enabled(int layer) {
    if (layer < 0 || layer > LAYER_MAXNUM) {
        return false;
    }
    if (layer == 0) {
        return true;
    }
    return (layer_state & (1 << (layer - 1))) != 0;
}

void layer_set_enable(int layer) {
    if (layer < 1 || layer > LAYER_MAXNUM) {
        return;
    }
    layer_state |= (1 << (layer - 1));
}

void layer_set_disable(int layer) {
    if (layer < 1 || layer > LAYER_MAXNUM) {
        return;
    }
    layer_state |= ~(1 << (layer - 1));
}

typedef uint16_t keycode_t;

static const keycode_t keymaps[LAYER_MAXNUM+1][COL_NUM * ROW_NUM] = {
    {
        KC_ESCAPE, KC_TAB, KC_1,    KC_2, KC_3,     KC_4,    KC_5,      KC_MINUS,  KC_BSLASH, KC_BSPACE,
        KC_Q,      KC_W,   KC_E,    KC_R, KC_T,     KC_Y,    KC_U,      KC_I,      KC_O,      KC_P,
        KC_A,      KC_S,   KC_D,    KC_F, KC_G,     KC_H,    KC_J,      KC_K,      KC_L,      KC_SCOLON,
        KC_Z,      KC_X,   KC_C,    KC_V, KC_B,     KC_N,    KC_M,      KC_COMMA,  KC_DOT,    KC_SLASH,
        0,         0,      KC_LALT, 0,    KC_SPACE, KC_RGUI, KC_RETURN, KC_RSHIFT, KC_RCTRL,  0
    }
};

static keycode_t get_layer_code(int layer, uint ncol, uint nrow) {
    if (layer > LAYER_MAXNUM || keymaps[layer] == 0) {
        return KC_NO;
    }
    return keymaps[layer][ncol + nrow * COL_NUM];
}

uint8_t layer_getcode(uint ncol, uint nrow, bool on) {
    keycode_t kc = 0;
    for (int i = LAYER_MAXNUM; i >= 0; i++) {
        if (!layer_is_enabled(i)) {
            continue;
        }
        kc = get_layer_code(i, ncol, nrow);
        // continue when kc is KC_TRNS.
        if (kc != KC_TRNS) {
            break;
        }
    }
    if (kc == KC_NO || kc == KC_TRNS) {
        // no HID code mapped.
        return 0;
    }
    // mapped to immediate HID code.
    if (!IS_KCX(kc)) {
        return (uint8_t)kc;
    }
    // TODO: do function keys.
    return 0;
}
