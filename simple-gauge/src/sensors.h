#pragma once
#include <lvgl.h>

// LVGL label pointers are defined in main.cpp but shared here for update routines
extern lv_obj_t *label_egt;
extern lv_obj_t *label_coolant;
extern lv_obj_t *label_boost;

void sensors_setup();
void sensors_update();
