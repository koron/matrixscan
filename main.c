#include <stdio.h>

#include "pico/stdlib.h"
#include "bsp/board.h"
#include "tusb.h"

#include "matrixscan.h"

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    // TODO not Implemented
    printf("tud_hid_get_report_cb: id=%d type=%d buf=%p len=%d\n", report_id, report_type, buffer, reqlen);
    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    // TODO set LED based on CAPLOCK, NUMLOCK etc...
    printf("tud_hid_set_report_cb: id=%d type=%d buf=%p size=%d\n", report_id, report_type, buffer, bufsize);
}

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
