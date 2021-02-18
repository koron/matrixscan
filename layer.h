#pragma once

extern uint32_t layer_state;

#ifdef __cplusplus
extern "C" {
#endif

bool layer_is_enabled(int layer);
void layer_set_enable(int layer);
void layer_set_disable(int layer);

uint8_t layer_getcode(uint ncol, uint nrow, bool on);

#ifdef __cplusplus
}
#endif
