#include <stdio.h>

#include "pico/stdlib.h"
#include "tusb.h"

#include "config.h"
#include "usb_descriptors.h"
#include "keycodes.h"

#define COUNT_OF(x) (sizeof(x) / sizeof(x[0]))

static bool hidkb_changed = false;
static uint8_t hidkb_mod = 0;
static uint8_t hidkb_code[6] = {0};

static uint8_t hidkb_code2mod(uint8_t code) {
    uint8_t mod = 0;
    switch (code) {
        case KC_LCTRL:  // HID_KEY_CONTROL_LEFT
            mod = KEYBOARD_MODIFIER_LEFTCTRL;
            break;
        case KC_LSHIFT: // HID_KEY_SHIFT_LEFT
            mod = KEYBOARD_MODIFIER_LEFTSHIFT;
            break;
        case KC_LALT:   // HID_KEY_ALT_LEFT
            mod = KEYBOARD_MODIFIER_LEFTALT;
            break;
        case KC_LGUI:   // HID_KEY_GUI_LEFT
            mod = KEYBOARD_MODIFIER_LEFTGUI;
            break;
        case KC_RCTRL:  // HID_KEY_CONTROL_RIGHT
            mod = KEYBOARD_MODIFIER_RIGHTCTRL;
            break;
        case KC_RSHIFT: // HID_KEY_SHIFT_RIGHT
            mod = KEYBOARD_MODIFIER_RIGHTSHIFT;
            break;
        case KC_RALT:   // HID_KEY_ALT_RIGHT
            mod = KEYBOARD_MODIFIER_RIGHTALT;
            break;
        case KC_RGUI:   // HID_KEY_GUI_RIGHT
            mod = KEYBOARD_MODIFIER_RIGHTGUI;
            break;
    }
    return mod;
}

// hidkb_report_code composes keyboard report which will be send.
void hidkb_report_code(uint8_t code, bool on) {
    //printf("hidkb_report_code: %02X %s\n", code, on ? "ON" : "OFF");
    // update modifier states.
    uint8_t mod = hidkb_code2mod(code);
    if (mod != 0) {
        if (on) {
            hidkb_mod |= mod;
        } else {
            hidkb_mod &= ~mod;
        }
        hidkb_changed |= true;
        return;
    }
    // update hidkb_code.
    int found = -1, vacant = -1;
    for (int i = 0; i < COUNT_OF(hidkb_code); i++) {
        if (vacant < 0 && hidkb_code[i] == 0) {
            vacant = i;
        }
        if (found < 0 && hidkb_code[i] == code) {
            found = i;
        }
    }
    // when key up.
    if (!on) {
        if (found >= 0) {
            hidkb_code[found] = 0;
            hidkb_changed |= true;
        }
        return;
    }
    // when key down.
    if (found >= 0 || vacant < 0) {
        return;
    }
    hidkb_code[vacant] = code;
    hidkb_changed |= true;
}

static void hidkb_task() {
    if (!hidkb_changed) {
        return;
    }
    // clean up hidkb_code. remove intermediate zeros, padding non-zero
    // codes to left.
    bool aligned = false;
    uint8_t tmp[6] = {0};
    for (int i = 0, j = 0; i < COUNT_OF(hidkb_code); i++) {
        if (hidkb_code[i] != 0) {
            tmp[j] = hidkb_code[i];
            if (j != i) {
                aligned |= true;
            }
            j++;
        }
    }
    if (aligned) {
        memcpy(hidkb_code, tmp, sizeof(hidkb_code));
    }
    // send keyboard report with boot protocol.
    //printf("hid_task: keyboard: %02X (%02X %02X %02X %02X %02X %02X)\n", hidkb_mod, hidkb_code[0], hidkb_code[1], hidkb_code[2], hidkb_code[3], hidkb_code[4], hidkb_code[5]);
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, hidkb_mod, hidkb_code);
    // clear changed flag.
    hidkb_changed = false;
}

void hid_task() {
    hidkb_task();
}

// Invoked when received GET_REPORT control request
//
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request.
__attribute__((weak)) uint16_t hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    printf("hid_get_report_cb(weak stub): id=%d type=%d buf=%p len=%d\n", report_id, report_type, buffer, reqlen);
    return 0;
}

uint16_t tud_hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    return hid_get_report_cb(report_id, report_type, buffer, reqlen);
}

// Invoked when received SET_REPORT control request or received data on OUT
// endpoint ( Report ID = 0, Type = 0 )
__attribute__((weak)) void hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    printf("hid_set_report_cb(weak stub): id=%d type=%d buf=%p size=%d\n", report_id, report_type, buffer, bufsize);
}

void tud_hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    hid_set_report_cb(report_id, report_type, buffer, bufsize);
}
