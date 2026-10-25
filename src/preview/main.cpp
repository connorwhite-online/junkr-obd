// Host renderer for the gauge pages: runs the real ui.cpp and gauge model
// on the PC against the demo bus traffic, and writes one PPM per page.
//
//   pio run -e gauge-preview -t exec          (writes .pio/gauge-preview/*.ppm)
//   python3 docs/gauge/firmware/convert.py    (PPM -> round-masked PNG)
#include <lvgl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "demo.h"
#include "gauge_state.h"
#include "ui.h"

namespace {

constexpr int W = 480;
lv_color_t fb[W * W];
lv_color_t buf[W * 40];
lv_disp_draw_buf_t draw_buf;
lv_disp_drv_t drv;

void flush(lv_disp_drv_t* d, const lv_area_t* a, lv_color_t* px) {
    for (int y = a->y1; y <= a->y2; ++y)
        memcpy(&fb[y * W + a->x1], px + (y - a->y1) * (a->x2 - a->x1 + 1),
               (a->x2 - a->x1 + 1) * sizeof(lv_color_t));
    lv_disp_flush_ready(d);
}

void save(const char* name) {
    char path[128];
    snprintf(path, sizeof path, ".pio/gauge-preview/%s.ppm", name);
    FILE* f = fopen(path, "wb");
    fprintf(f, "P6\n%d %d\n255\n", W, W);
    for (int i = 0; i < W * W; ++i) {
        const uint16_t c = fb[i].full;
        const uint8_t rgb[3] = {(uint8_t)(((c >> 11) & 0x1F) * 255 / 31), (uint8_t)(((c >> 5) & 0x3F) * 255 / 63),
                                (uint8_t)((c & 0x1F) * 255 / 31)};
        fwrite(rgb, 1, 3, f);
    }
    fclose(f);
    printf("wrote %s\n", path);
}

gauge::State state;
uint32_t now = 0;

// Advance the simulated truck to `t_ms`, at 20 Hz like the firmware.
void run_to(uint32_t t_ms) {
    can_proto::Frame f[8];
    for (; now < t_ms; now += 50) {
        const int n = demo::frames(now, f);
        for (int i = 0; i < n; ++i) state.ingest(f[i].id, f[i].data, f[i].len, now);
        state.tick(now);
    }
}

void render(const char* name) {
    ui_update(state, now);
    lv_tick_inc(100);
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
    save(name);
}

}  // namespace

int main() {
    mkdir(".pio", 0755);
    mkdir(".pio/gauge-preview", 0755);
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, W * 40);
    lv_disp_drv_init(&drv);
    drv.hor_res = W;
    drv.ver_res = W;
    drv.flush_cb = flush;
    drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&drv);
    ui_init();

    // Seven demo loops fill the 10-minute coolant trend; t = 30 s into a
    // loop is cruise with the tune armed.
    run_to(6 * demo::LOOP_MS + 30000);
    render("01_drive");
    state.short_press(now);
    render("02_temps");
    state.short_press(now);
    render("03_engine");
    state.short_press(now);
    render("04_tune");
    state.short_press(now);

    run_to(6 * demo::LOOP_MS + 47000);   // the hill pull, before EGT trips
    render("06_drive_pull");
    run_to(6 * demo::LOOP_MS + 54500);   // EGT over 1250 °F for 2 s
    render("05_alert");
    return 0;
}
