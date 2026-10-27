// Temperature knob logic for the climate panel.
//
// The stock A/C amplifier only understands its TEMP up / TEMP down buttons,
// so the knob "presses" them: one detent = one press, sent through a
// photoMOS relay wired across the stock switch. The amplifier never tells
// us its setpoint, so we keep our own copy and can re-sync it by pressing
// DOWN past LO and counting back up (homing).
//
// Pure logic (fed detents and time), so the host tests cover it.
#pragma once
#include <stdint.h>

namespace climate {

// Toyota auto A/C setpoints: LO, 18.0 .. 32.0 °C in 0.5 °C steps, HI.
// Index 0 = LO, 1 = 18.0, ..., 29 = 32.0, 30 = HI. [VERIFY on the truck]
struct Scale {
    static constexpr int MIN_X2 = 36;   // 18.0 °C
    static constexpr int MAX_X2 = 64;   // 32.0 °C
    static constexpr int STEPS = MAX_X2 - MIN_X2 + 2;   // LO .. HI = 30

    static bool is_lo(int i) { return i <= 0; }
    static bool is_hi(int i) { return i >= STEPS; }
    // Setpoint in tenths of °C; only valid when neither LO nor HI.
    static int deci_c(int i) { return (MIN_X2 + i - 1) * 5; }
    static int deci_f(int i) { return (deci_c(i) * 9 + 1600) / 5; }
};

class Knob {
  public:
    enum Out : uint8_t { IDLE, UP, DOWN };

    static constexpr uint32_t PULSE_MS = 90;    // press length the amplifier must see [VERIFY]
    static constexpr uint32_t GAP_MS = 110;     // release before the next press [VERIFY]

    explicit Knob(int start = 15)   // 25.0 °C, the amplifier's reset value [VERIFY]
         : target_(clamp(start)), assumed_(target_) {}

    // Turn by n detents (+ = warmer).
    void turn(int n) { target_ = clamp(target_ + n); }
    void set_target(int i) { target_ = clamp(i); }

    // Forget what the amplifier holds: press DOWN past LO, then climb back
    // to the target. Use after a battery disconnect or a missed press.
    void home() {
        force_down_ = Scale::STEPS + 2;
        assumed_ = Scale::STEPS;
    }

    // Call every few ms. Returns which relay should be closed right now.
    Out tick(uint32_t now) {
        if (out_ != IDLE) {
            if (now - t_ < PULSE_MS) return out_;
            // press done: the amplifier moved one step
            if (out_ == UP) assumed_ = clamp(assumed_ + 1);
            else assumed_ = clamp(assumed_ - 1);
            if (force_down_ > 0 && --force_down_ == 0) assumed_ = 0;
            out_ = IDLE;
            t_ = now;
            return IDLE;
        }
        if (now - t_ < GAP_MS) return IDLE;
        Out next = IDLE;
        if (force_down_ > 0) next = DOWN;
        else if (target_ > assumed_) next = UP;
        else if (target_ < assumed_) next = DOWN;
        if (next != IDLE) {
            out_ = next;
            t_ = now;
        }
        return out_;
    }

    int target() const { return target_; }
    int assumed() const { return assumed_; }
    bool syncing() const { return force_down_ > 0; }
    bool settled() const { return out_ == IDLE && force_down_ == 0 && target_ == assumed_; }

  private:
    static int clamp(int i) { return i < 0 ? 0 : (i > Scale::STEPS ? Scale::STEPS : i); }

    int target_, assumed_;
    int force_down_ = 0;
    Out out_ = IDLE;
    uint32_t t_ = 0;
};

}  // namespace climate
