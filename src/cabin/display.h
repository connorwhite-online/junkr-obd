#pragma once
#include <stdint.h>

// Qualia ESP32-S3 + 2.1" round 480 x 480 TL021WVC02 (RGB666 parallel),
// wired into LVGL. Also owns the board's I/O expander, so the on-board
// buttons are read here.
bool display_setup();
bool display_button_down();   // Qualia "DOWN" button (active low on the expander)
