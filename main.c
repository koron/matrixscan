#include <stdio.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

#include "config.h"
#include "matrixscan.h"
#include "layer.h"
#include "hid.h"

#if 1 // HID report customization.

// Invoked when received GET_REPORT control request.
//
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request.
uint16_t hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    // TODO: implement me.
    //printf("hid_get_report_cb: id=%d type=%d buf=%p len=%d\n", report_id, report_type, buffer, reqlen);
    return 0;
}

// Invoked when received SET_REPORT control request or received data on OUT
// endpoint ( Report ID = 0, Type = 0 )
void hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    // TODO: set LED based on CAPLOCK, NUMLOCK etc...
    //printf("hid_set_report_cb: id=%d type=%d buf=%p size=%d\n", report_id, report_type, buffer, bufsize);
}

#endif // HID report customization.

#if 1 // Link matrixscan to layer and hid.

void matrix_changed(uint ncol, uint nrow, bool on, uint8_t when) {
    //printf("matrix_changed: col=%d row=%d %s when=%d\n", ncol, nrow, on ? "ON" : "OFF", when);
    uint8_t code = layer_getcode(ncol, nrow, on);
    if (code != 0) {
        hidkb_report_code(code, on);
    }
}

void matrix_suppressed(uint ncol, uint nrow, bool on, uint8_t when, uint8_t last, uint8_t elapsed) {
    //printf("matrix_suppressed: col=%d row=%d %s when=%d last=%d elapsed=%d\n", ncol, nrow, on ? "ON" : "OFF", when, last, elapsed);
}

#endif // Link matrixscan to layer and hid.

int main()
{
    board_init();
    tusb_init();
    stdio_init_all();
    matrix_init();

    printf("Prototype Keyboard start\n");

    while (true) {
        tud_task(); // tinyusb device task
        matrix_task();
        hid_task();
    }

    return 0;
}
