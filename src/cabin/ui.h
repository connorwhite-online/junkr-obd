#pragma once
#include <stdint.h>

#include "gauge_state.h"

// LVGL screens for the 480 x 480 round gauge. Needs only LVGL, so the
// host preview (env gauge-preview) renders the very same pages.
void ui_init();
void ui_update(const gauge::State& s, uint32_t now);
