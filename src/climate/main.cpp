// Climate panel temperature knob: M5Stack Dial (ESP32-S3, round screen,
// encoder) presses the stock panel's TEMP up/down switches through photoMOS
// relays. Every other button on the new panel is wired straight to its stock
// switch and never passes through here.
//
//   PORT.B G1 -> photoMOS across TEMP UP    (330 R to the relay LED)
//   PORT.B G2 -> photoMOS across TEMP DOWN
//   PORT.A G13 -> photoMOS across AUTO     (screen press = AUTO)
//
// Turn: warmer / cooler. Press the screen: AUTO. Hold 2 s: re-sync (homing).
#include <M5Dial.h>
#include <Preferences.h>

#include "climate_logic.h"

namespace {

constexpr int PIN_UP = 1;
constexpr int PIN_DOWN = 2;
constexpr int PIN_AUTO = 13;
constexpr uint32_t AUTO_PULSE_MS = 120;
constexpr uint32_t HOLD_HOME_MS = 2000;
constexpr uint32_t SAVE_AFTER_MS = 3000;   // write flash once the knob rests
constexpr int COUNTS_PER_DETENT = 4;        // 64 pulses / 16 detents
constexpr bool FAHRENHEIT = false;

climate::Knob knob;
Preferences prefs;
M5Canvas canvas(&M5Dial.Display);

long enc_last = 0;
uint32_t t_turn = 0, t_auto = 0;
bool dirty = false, auto_on = false, home_latched = false;
int drawn = -1;
bool drawn_sync = false;

void relay(climate::Knob::Out o) {
    digitalWrite(PIN_UP, o == climate::Knob::UP);
    digitalWrite(PIN_DOWN, o == climate::Knob::DOWN);
}

uint32_t colour_for(int i) {
    // blue at LO, white in the middle, red at HI
    float f = (float)i / climate::Scale::STEPS;
    uint8_t r = f < 0.5f ? (uint8_t)(80 + 350 * f) : 255;
    uint8_t b = f > 0.5f ? (uint8_t)(255 - 350 * (f - 0.5f)) : 255;
    uint8_t g = (uint8_t)(255 - 300 * (f > 0.5f ? f - 0.5f : 0.5f - f));
    return M5Dial.Display.color888(r, g, b);
}

void draw() {
    int i = knob.target();
    if (i == drawn && knob.syncing() == drawn_sync) return;
    drawn = i;
    drawn_sync = knob.syncing();

    canvas.fillSprite(TFT_BLACK);
    // position arc, 240° sweep
    float a1 = 150 + 240.0f * i / climate::Scale::STEPS;
    canvas.fillArc(120, 120, 116, 108, 150, 390, 0x2104);
    canvas.fillArc(120, 120, 116, 108, 150, a1, colour_for(i));

    char buf[8];
    using S = climate::Scale;
    if (S::is_lo(i)) snprintf(buf, sizeof buf, "LO");
    else if (S::is_hi(i)) snprintf(buf, sizeof buf, "HI");
    else {
        int d = FAHRENHEIT ? S::deci_f(i) : S::deci_c(i);
        snprintf(buf, sizeof buf, "%d.%d", d / 10, d % 10);
    }
    canvas.setTextDatum(middle_center);
    canvas.setTextColor(TFT_WHITE);
    canvas.setFont(&fonts::FreeSansBold24pt7b);
    canvas.drawString(buf, 120, 112);
    canvas.setFont(&fonts::FreeSans9pt7b);
    canvas.setTextColor(0x9CF3);
    canvas.drawString(knob.syncing() ? "syncing..." : (FAHRENHEIT ? "F" : "C"), 120, 160);
    canvas.pushSprite(0, 0);
}

}  // namespace

void setup() {
    auto cfg = M5.config();
    M5Dial.begin(cfg, true, false);   // encoder on, RFID off
    pinMode(PIN_UP, OUTPUT);
    pinMode(PIN_DOWN, OUTPUT);
    pinMode(PIN_AUTO, OUTPUT);
    relay(climate::Knob::IDLE);
    digitalWrite(PIN_AUTO, LOW);

    canvas.createSprite(240, 240);
    M5Dial.Display.setBrightness(120);

    // The amplifier keeps its setpoint on +B, and so do we (in flash). If
    // they ever disagree (battery off), hold the screen 2 s to re-sync.
    prefs.begin("climate", false);
    int saved = prefs.getInt("target", -1);
    if (saved < 0) {
        knob.home();   // first boot ever: find LO, then go to 25.0
        dirty = true;
    } else {
        knob = climate::Knob(saved);
    }
    enc_last = M5Dial.Encoder.read();
    draw();
}

void loop() {
    M5Dial.update();
    uint32_t now = millis();

    long enc = M5Dial.Encoder.read();
    long detents = (enc - enc_last) / COUNTS_PER_DETENT;
    if (detents != 0) {
        enc_last += detents * COUNTS_PER_DETENT;
        knob.turn((int)detents);
        t_turn = now;
        dirty = true;
    }

    if (M5Dial.BtnA.isReleased()) home_latched = false;
    if (M5Dial.BtnA.pressedFor(HOLD_HOME_MS) && !home_latched) {
        home_latched = true;
        knob.home();
        dirty = true;
        M5Dial.Speaker.tone(2000, 60);
    } else if (M5Dial.BtnA.wasClicked()) {
        auto_on = true;
        t_auto = now;
    }
    digitalWrite(PIN_AUTO, auto_on && now - t_auto < AUTO_PULSE_MS);
    if (auto_on && now - t_auto >= AUTO_PULSE_MS) auto_on = false;

    relay(knob.tick(now));
    draw();

    if (dirty && now - t_turn > SAVE_AFTER_MS && knob.settled()) {
        prefs.putInt("target", knob.target());
        dirty = false;
    }
    delay(2);
}
