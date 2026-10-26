#pragma once
#include <stdint.h>

void sensors_setup();

int16_t read_egt_f();   // °F; SENTINEL_INVALID on thermocouple fault

// Volts at the J3 / J4 connectors (the on-board 2:1 dividers undone).
float read_map_v();     // ADS1115 A1, 3-bar MAP signal (every cycle)
float read_aux_v();     // ADS1115 A2, spare (oil pressure later)
float read_v5_v();      // ADS1115 A3, +5 V sensor supply monitor
