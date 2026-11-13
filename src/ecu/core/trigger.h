// NE (pump speed) and TDC (crank) decoding for the Denso ECD-V3 pump.
//
// The NE pulsar rides the pump's roller ring: 64 slots per pump turn
// (one pump turn = 720 deg CA), 52 teeth, with 3 teeth missing at 4 places.
// So every 180 deg CA there are 13 teeth 11.25 deg CA apart, then a gap
// spanning 4 tooth pitches. Tooth 0 is the first tooth after a gap.
//
// Because the sensor moves with the roller ring, NE angle is plunger angle:
// fuel (SPV spill) is scheduled in NE angle. The TDC sensor is on the crank,
// so the TDC edge's position in the NE segment measures the actual injection
// advance (the timer piston turns the roller ring).
#pragma once
#include <stdint.h>

namespace ecu {

constexpr int NE_TEETH_PER_SEG = 13;
constexpr int NE_GAP_PITCHES = 4;          // gap spans 4 pitches (3 missing teeth)
constexpr float NE_PITCH_DEG = 11.25f;     // crank degrees per tooth pitch
constexpr float NE_SEG_DEG = 180.0f;       // one segment per cylinder (4 cyl)

class NeDecoder {
public:
    // Call on every NE edge (the comparator edge that marks a tooth).
    void tooth(uint32_t t_us) {
        if (!have_last_) { have_last_ = true; last_ = t_us; return; }
        uint32_t dt = t_us - last_;
        last_ = t_us;
        // A gap is about 4x a normal pitch; 2.6x leaves room for cranking acceleration.
        bool gap = prev_dt_ && dt * 10 > prev_dt_ * 26;
        if (!synced_) {
            if (gap) { synced_ = true; tooth_ = 0; seg_start_ = t_us; seg_valid_ = false; }
            else prev_dt_ = dt;
            if (gap) prev_dt_ = dt / NE_GAP_PITCHES;
            return;
        }
        int next = tooth_ + 1;
        if (next == NE_TEETH_PER_SEG) {
            // The gap must come here.
            if (!gap) { lose_sync(); return; }
            seg_period_ = t_us - seg_start_;
            seg_valid_ = true;
            seg_start_ = t_us;
            segments_++;
            tooth_ = 0;
            prev_dt_ = dt / NE_GAP_PITCHES;
            return;
        }
        if (gap) { lose_sync(); return; }
        // A tooth far too early (noise) also breaks sync.
        if (dt * 10 < prev_dt_ * 4) { lose_sync(); return; }
        tooth_ = next;
        prev_dt_ = dt;
    }

    bool synced() const { return synced_; }
    bool rpm_valid() const { return synced_ && seg_valid_; }
    int tooth_index() const { return tooth_; }
    uint32_t segments() const { return segments_; }
    uint32_t sync_losses() const { return losses_; }
    uint32_t last_tooth_us() const { return last_; }
    uint32_t segment_start_us() const { return seg_start_; }
    // Time of one tooth pitch, from the latest normal tooth.
    uint32_t pitch_us() const { return prev_dt_; }
    // Engine (crank) rpm, from the last whole segment (180 deg CA).
    float rpm() const { return rpm_valid() ? 30.0e6f / (float)seg_period_ : 0.0f; }
    // Instantaneous rpm from the last pitch.
    float rpm_inst() const { return synced_ && prev_dt_ ? (NE_PITCH_DEG / 360.0f) * 60.0e6f / (float)prev_dt_ : 0.0f; }
    // NE angle (deg CA after tooth 0) at time t, extrapolated from the last tooth.
    float angle_at(uint32_t t_us) const {
        if (!synced_ || !prev_dt_) return -1;
        // Signed: a capture timestamp can be a little older than the last tooth
        // when two ISRs race, and that must not wrap around.
        int32_t d = (int32_t)(t_us - last_);
        float a = tooth_ * NE_PITCH_DEG + NE_PITCH_DEG * (float)d / (float)prev_dt_;
        return a;
    }
    // Stalled: no tooth for `timeout_us`.
    bool stalled(uint32_t now_us, uint32_t timeout_us = 200000) const {
        return !have_last_ || (now_us - last_) > timeout_us;
    }
    void reset() { *this = NeDecoder(); }

private:
    void lose_sync() { synced_ = false; seg_valid_ = false; losses_++; prev_dt_ = 0; }
    bool have_last_ = false, synced_ = false, seg_valid_ = false;
    uint32_t last_ = 0, prev_dt_ = 0, seg_start_ = 0, seg_period_ = 0;
    uint32_t segments_ = 0, losses_ = 0;
    int tooth_ = 0;
};

// Phase of the crank TDC edge inside the NE segment, in deg CA after tooth 0.
// advance = phase - cal_offset: more advance turns the roller ring so the NE
// teeth arrive earlier, and the TDC edge lands later in the segment.
// cal_offset (and the sign) are found on the bench, in shadow mode against
// the stock ECU or with a timing light on #1. [VERIFY]
class TdcPhase {
public:
    void edge(uint32_t t_us, const NeDecoder &ne) {
        if (!ne.rpm_valid()) { valid_ = false; return; }
        float a = ne.angle_at(t_us);
        if (a < 0 || a >= NE_SEG_DEG) return;   // implausible: ignore the edge
        // Smooth a little: the timer piston moves slowly anyway.
        phase_ = valid_ ? phase_ + 0.3f * (a - phase_) : a;
        valid_ = true;
        count_++;
    }
    bool valid() const { return valid_; }
    float phase_deg() const { return phase_; }
    float advance_deg(float cal_offset) const { return phase_ - cal_offset; }
    uint32_t count() const { return count_; }

private:
    bool valid_ = false;
    float phase_ = 0;
    uint32_t count_ = 0;
};

}  // namespace ecu
