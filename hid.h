#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void hid_task();
void hidkb_report_code(uint8_t code, bool on);

#ifdef __cplusplus
}
#endif
