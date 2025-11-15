#pragma once
#include <lvgl.h>

extern lv_obj_t *label_egt;
extern lv_obj_t *label_coolant;
extern lv_obj_t *label_boost;

void sensors_setup();
void sensors_update();
