#pragma once
#include "gauge_state.h"

bool can_rx_setup();
void can_rx_poll(gauge::State& s, uint32_t now);
