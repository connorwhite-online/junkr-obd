// LVGL 8.3 configuration for the round gauge. Anything not set here keeps
// LVGL's default (see lv_conf_internal.h).
#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

// Plain malloc: on the Qualia large blocks land in PSRAM automatically.
#define LV_MEM_CUSTOM 1

#ifdef ARDUINO
#define LV_TICK_CUSTOM 1
#define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
#define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#else
#define LV_TICK_CUSTOM 0  // host preview drives lv_tick_inc() itself
#endif

#define LV_DISP_DEF_REFR_PERIOD 33   // ~30 fps is plenty for a gauge
#define LV_DPI_DEF 230               // 2.1" 480 x 480

#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1

#define LV_USE_CHART 1
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_LINE 1
#define LV_USE_LABEL 1

#endif  // LV_CONF_H
