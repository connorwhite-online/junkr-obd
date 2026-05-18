#include "ui.h"
#include "can_protocol.h"

#include <lvgl.h>
#include <stdio.h>

namespace {

lv_obj_t *boost_arc;
lv_obj_t *boost_value_lbl;
lv_obj_t *boost_unit_lbl;
lv_obj_t *egt_value_lbl;
lv_obj_t *egt_caption_lbl;
lv_obj_t *coolant_value_lbl;
lv_obj_t *coolant_caption_lbl;

constexpr int16_t EGT_WARN_F        = 1300;
constexpr int16_t EGT_DANGER_F      = 1500;
constexpr int16_t COOLANT_WARN_F    = 220;
constexpr int16_t COOLANT_DANGER_F  = 240;
constexpr int16_t BOOST_WARN_X10    = 200;  // 20.0 psi
constexpr int16_t BOOST_DANGER_X10  = 270;  // 27.0 psi

lv_color_t threshold_color(int16_t value, int16_t warn, int16_t danger) {
  if (value >= danger) return lv_palette_main(LV_PALETTE_RED);
  if (value >= warn)   return lv_palette_main(LV_PALETTE_AMBER);
  return lv_color_white();
}

}  // namespace

void ui_init() {
  lv_obj_t *scr = lv_scr_act();
  lv_obj_set_style_bg_color(scr, lv_color_black(), LV_PART_MAIN);

  const int hres = lv_disp_get_hor_res(NULL);
  const int vres = lv_disp_get_ver_res(NULL);

  // ---- Boost arc (top, centered) ---------------------------------------
  const int arc_size = (hres * 7) / 10;
  boost_arc = lv_arc_create(scr);
  lv_obj_set_size(boost_arc, arc_size, arc_size);
  lv_obj_align(boost_arc, LV_ALIGN_TOP_MID, 0, vres / 40);

  lv_arc_set_range(boost_arc, 0, 290);          // 0.0–29.0 psi (×10)
  lv_arc_set_bg_angles(boost_arc, 135, 45);     // 270° sweep, bottom-open
  lv_arc_set_rotation(boost_arc, 0);
  lv_arc_set_value(boost_arc, 0);

  lv_obj_remove_style(boost_arc, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(boost_arc, LV_OBJ_FLAG_CLICKABLE);

  lv_obj_set_style_arc_width(boost_arc, 16, LV_PART_MAIN);
  lv_obj_set_style_arc_width(boost_arc, 20, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(boost_arc,
      lv_palette_darken(LV_PALETTE_GREY, 3), LV_PART_MAIN);
  lv_obj_set_style_arc_color(boost_arc,
      lv_palette_main(LV_PALETTE_CYAN), LV_PART_INDICATOR);

  boost_value_lbl = lv_label_create(scr);
  lv_label_set_text(boost_value_lbl, "0.0");
  lv_obj_set_style_text_color(boost_value_lbl, lv_color_white(), 0);
  lv_obj_set_style_text_font(boost_value_lbl, &lv_font_montserrat_48, 0);
  lv_obj_align_to(boost_value_lbl, boost_arc, LV_ALIGN_CENTER, 0, -8);

  boost_unit_lbl = lv_label_create(scr);
  lv_label_set_text(boost_unit_lbl, "psi");
  lv_obj_set_style_text_color(boost_unit_lbl,
      lv_palette_lighten(LV_PALETTE_GREY, 2), 0);
  lv_obj_align_to(boost_unit_lbl, boost_value_lbl,
      LV_ALIGN_OUT_BOTTOM_MID, 0, 2);

  // ---- EGT (bottom-left numeric) ---------------------------------------
  const int margin = hres / 24;

  egt_value_lbl = lv_label_create(scr);
  lv_label_set_text(egt_value_lbl, "---");
  lv_obj_set_style_text_color(egt_value_lbl, lv_color_white(), 0);
  lv_obj_set_style_text_font(egt_value_lbl, &lv_font_montserrat_36, 0);
  lv_obj_align(egt_value_lbl, LV_ALIGN_BOTTOM_LEFT, margin, -(vres / 16));

  egt_caption_lbl = lv_label_create(scr);
  lv_label_set_text(egt_caption_lbl, "EGT  F");
  lv_obj_set_style_text_color(egt_caption_lbl,
      lv_palette_lighten(LV_PALETTE_GREY, 2), 0);
  lv_obj_align(egt_caption_lbl, LV_ALIGN_BOTTOM_LEFT, margin, -(vres / 64));

  // ---- Coolant (bottom-right numeric) ----------------------------------
  coolant_value_lbl = lv_label_create(scr);
  lv_label_set_text(coolant_value_lbl, "---");
  lv_obj_set_style_text_color(coolant_value_lbl, lv_color_white(), 0);
  lv_obj_set_style_text_font(coolant_value_lbl, &lv_font_montserrat_36, 0);
  lv_obj_align(coolant_value_lbl, LV_ALIGN_BOTTOM_RIGHT, -margin, -(vres / 16));

  coolant_caption_lbl = lv_label_create(scr);
  lv_label_set_text(coolant_caption_lbl, "COOLANT  F");
  lv_obj_set_style_text_color(coolant_caption_lbl,
      lv_palette_lighten(LV_PALETTE_GREY, 2), 0);
  lv_obj_align(coolant_caption_lbl, LV_ALIGN_BOTTOM_RIGHT, -margin, -(vres / 64));
}

void ui_set_boost_x10(int16_t psi_x10) {
  if (psi_x10 == can_proto::SENTINEL_INVALID) {
    lv_label_set_text(boost_value_lbl, "--");
    lv_arc_set_value(boost_arc, 0);
    return;
  }
  if (psi_x10 < 0) psi_x10 = 0;
  lv_arc_set_value(boost_arc, psi_x10);

  char buf[8];
  snprintf(buf, sizeof(buf), "%d.%d", psi_x10 / 10, psi_x10 % 10);
  lv_label_set_text(boost_value_lbl, buf);

  lv_color_t c = threshold_color(psi_x10, BOOST_WARN_X10, BOOST_DANGER_X10);
  lv_obj_set_style_text_color(boost_value_lbl, c, 0);
  lv_obj_set_style_arc_color(boost_arc, c, LV_PART_INDICATOR);
}

void ui_set_egt(int16_t f) {
  if (f == can_proto::SENTINEL_INVALID) {
    lv_label_set_text(egt_value_lbl, "---");
    lv_obj_set_style_text_color(egt_value_lbl, lv_color_white(), 0);
    return;
  }
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", f);
  lv_label_set_text(egt_value_lbl, buf);
  lv_obj_set_style_text_color(egt_value_lbl,
      threshold_color(f, EGT_WARN_F, EGT_DANGER_F), 0);
}

void ui_set_coolant(int16_t f) {
  if (f == can_proto::SENTINEL_INVALID) {
    lv_label_set_text(coolant_value_lbl, "---");
    lv_obj_set_style_text_color(coolant_value_lbl, lv_color_white(), 0);
    return;
  }
  char buf[8];
  snprintf(buf, sizeof(buf), "%d", f);
  lv_label_set_text(coolant_value_lbl, buf);
  lv_obj_set_style_text_color(coolant_value_lbl,
      threshold_color(f, COOLANT_WARN_F, COOLANT_DANGER_F), 0);
}
