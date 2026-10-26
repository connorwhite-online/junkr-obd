// One momentary button: debounced, short press vs long press.
// Pure logic (fed a raw level + time), so the host tests cover it.
#pragma once
#include <stdint.h>

namespace gauge {

class Button {
  public:
    enum Event : uint8_t { NONE, SHORT, LONG };

    static constexpr uint32_t DEBOUNCE_MS = 30;
    static constexpr uint32_t LONG_MS = 800;

    // pressed: raw level, already inverted for active-low wiring.
    Event update(bool pressed, uint32_t now) {
        if (pressed != raw_) {
            raw_ = pressed;
            t_raw_ = now;
        }
        if (now - t_raw_ < DEBOUNCE_MS || raw_ == stable_) {
            if (stable_ && !long_sent_ && now - t_down_ >= LONG_MS) {
                long_sent_ = true;
                return LONG;
            }
            return NONE;
        }
        stable_ = raw_;
        if (stable_) {
            t_down_ = now;
            long_sent_ = false;
            return NONE;
        }
        return long_sent_ ? NONE : SHORT;   // released
    }

  private:
    bool raw_ = false, stable_ = false, long_sent_ = false;
    uint32_t t_raw_ = 0, t_down_ = 0;
};

}  // namespace gauge
