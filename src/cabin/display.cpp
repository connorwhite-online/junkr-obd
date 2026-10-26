#include "display.h"

#include <Arduino.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include <esp_heap_caps.h>
#include <lvgl.h>

namespace {

constexpr int W = 480, H = 480;
constexpr int BUF_LINES = 48;   // LVGL draws in 480 x 48 strips

// Panel timings and init sequence from Adafruit's Qualia examples for the
// 2.1" round TL021WVC02 (PID 5792 / 5806).
Arduino_XCA9554SWSPI* expander = new Arduino_XCA9554SWSPI(
    PCA_TFT_RESET, PCA_TFT_CS, PCA_TFT_SCK, PCA_TFT_MOSI, &Wire, 0x3F);

Arduino_ESP32RGBPanel* rgbpanel = new Arduino_ESP32RGBPanel(
    TFT_DE, TFT_VSYNC, TFT_HSYNC, TFT_PCLK,
    TFT_R1, TFT_R2, TFT_R3, TFT_R4, TFT_R5,
    TFT_G0, TFT_G1, TFT_G2, TFT_G3, TFT_G4, TFT_G5,
    TFT_B1, TFT_B2, TFT_B3, TFT_B4, TFT_B5,
    1 /* hsync_polarity */, 50 /* hsync_front_porch */, 2 /* hsync_pulse_width */, 44 /* hsync_back_porch */,
    1 /* vsync_polarity */, 16 /* vsync_front_porch */, 2 /* vsync_pulse_width */, 18 /* vsync_back_porch */);

Arduino_RGB_Display* gfx = new Arduino_RGB_Display(
    W, H, rgbpanel, 0 /* rotation */, true /* auto_flush */,
    expander, GFX_NOT_DEFINED /* RST */, TL021WVC02_init_operations, sizeof(TL021WVC02_init_operations));

lv_disp_draw_buf_t draw_buf;
lv_disp_drv_t disp_drv;

void flush(lv_disp_drv_t* drv, const lv_area_t* a, lv_color_t* px) {
  const uint32_t w = a->x2 - a->x1 + 1, h = a->y2 - a->y1 + 1;
  gfx->draw16bitRGBBitmap(a->x1, a->y1, (uint16_t*)&px->full, w, h);
  lv_disp_flush_ready(drv);
}

}  // namespace

bool display_setup() {
  Wire.setClock(1000000);  // the expander bit-bangs the panel's SPI init
  if (!gfx->begin()) return false;
  gfx->fillScreen(0x0000);
  expander->pinMode(PCA_TFT_BACKLIGHT, OUTPUT);
  expander->digitalWrite(PCA_TFT_BACKLIGHT, HIGH);
  expander->pinMode(PCA_BUTTON_DOWN, INPUT);

  lv_init();
  const size_t n = W * BUF_LINES;
  auto* buf = (lv_color_t*)heap_caps_malloc(n * sizeof(lv_color_t), MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
  if (!buf) return false;
  lv_disp_draw_buf_init(&draw_buf, buf, nullptr, n);
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = W;
  disp_drv.ver_res = H;
  disp_drv.flush_cb = flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
  return true;
}

bool display_button_down() { return !expander->digitalRead(PCA_BUTTON_DOWN); }
