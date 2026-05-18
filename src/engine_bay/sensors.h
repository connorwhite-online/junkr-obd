#pragma once
#include <stdint.h>

void sensors_setup();

int16_t read_egt_f();          // °F; SENTINEL_INVALID on fault
int16_t read_coolant_f();      // °F; SENTINEL_INVALID on out-of-range
int16_t read_map_psi_x10();    // psi × 10
