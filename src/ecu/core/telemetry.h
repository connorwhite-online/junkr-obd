// Full-rate logging from the engine controller to the ESP32 (jlog format,
// shared/ecu_log.h). Host-testable: the HAL supplies a Tx that knows how
// much room its UART buffer has, and nothing here ever blocks. When the
// link can't keep up, frames are dropped and counted, never queued behind
// the control loop.
//
//   on_segment()  from the NE ISR at tooth 0: queues one INJ record
//   poll()        from the 1 kHz loop: samples at 100 Hz, drains the INJ
//                 queue, emits events, repeats SCHEMA and META every second
#pragma once
#include <stdio.h>
#include "ecu_log.h"
#include "engine.h"

namespace ecu {

struct LinkTx {
    virtual size_t room() = 0;                          // bytes that can be written now without blocking
    virtual void write(const uint8_t *p, size_t n) = 0;
    virtual ~LinkTx() {}
};

// Values only the HAL knows.
struct HalStatus {
    float stock_tcv_duty = 0;
    uint16_t mux[3] = {0, 0, 0};
    uint32_t loop_us = 0;
    bool arm = false;          // ARM readback (the AND gate's output)
    bool standalone = false;   // no stock ECU on the harness
};

inline uint32_t sample_flags(const Engine &e, const Sensors &s, const HalStatus &h) {
    using namespace jlog;
    const Commands &c = e.commands();
    uint32_t f = 0;
    if (e.ne().synced()) f |= F_SYNCED;
    if (e.ne().rpm_valid()) f |= F_RPM_VALID;
    if (c.spv.inject) f |= F_INJECT;
    if (c.takeover) f |= F_TAKEOVER;
    if (c.arm_req) f |= F_ARM_REQ;
    if (s.arm_switch) f |= F_ARM_SW;
    if (s.supervisor_ok) f |= F_SUPERVISOR_OK;
    if (h.arm) f |= F_ARM;
    if (s.idl) f |= F_IDL;
    if (s.brake) f |= F_BRAKE;
    if (s.starter) f |= F_STARTER;
    if (s.od_switch_on) f |= F_OD_SW;
    if (c.s1) f |= F_S1;
    if (c.s2) f |= F_S2;
    if (c.sl) f |= F_SL;
    if (c.svr_on) f |= F_SVR;
    if (e.timing().open_loop()) f |= F_TIMING_OPEN_LOOP;
    if (e.tdc().valid()) f |= F_TDC_VALID;
    if (h.standalone) f |= F_STANDALONE;
    if (e.timing().timing_fault()) f |= F_TIMING_FAULT;
    if (e.exhaust().egt_limiting()) f |= F_EGT_LIMIT;
    if (e.exhaust().wb_valid()) f |= F_WB_VALID;
    if (e.exhaust().learning()) f |= F_WB_LEARN;
    return f;
}

inline void fill_sample(jlog::Sample &x, uint32_t t_us, uint32_t seq, const Engine &e, const Calibration &cal,
                        const Sensors &s, const HalStatus &h) {
    using namespace jlog;
    const Commands &c = e.commands();
    x.begin(t_us, seq);
    x.set(C_RPM, e.ne().rpm());
    x.set(C_RPM_INST, e.ne().rpm_inst());
    x.set(C_Q, c.q);
    x.set(C_SPV_CLOSE, c.spv.close_deg);
    x.set(C_SPV_OPEN, c.spv.open_deg);
    x.set(C_ADV, e.tdc().advance_deg(cal.tdc_cal_offset));
    x.set(C_ADV_TARGET, e.timing().target());
    x.set(C_ADV_ERR, e.timing().error());
    x.set(C_TCV, c.tcv_duty);
    x.set(C_PEDAL, s.pedal);
    x.set(C_VA_V, s.va_v);
    x.set(C_MAP, s.map_kpa);
    x.set(C_MAP_V, s.map_v);
    x.set(C_ECT, s.ect_c);
    x.set(C_ECT_V, s.ect_v);
    x.set(C_VBATT, s.vbatt);
    x.set(C_SPEED, s.speed_kmh);
    x.set(C_RANGE, (float)(int)s.range);
    x.set(C_GEAR, (float)e.trans().gear());
    x.set(C_MODE, (float)(int)e.mode());
    x.set_u32(C_FLAGS, sample_flags(e, s, h));
    x.set_u32(C_DTC, c.dtc);
    x.set_u32(C_SYNC_LOSSES, e.ne().sync_losses());
    x.set_u32(C_AGREE_RUN, e.learner().agree_run());
    x.set(C_STOCK_TCV, h.stock_tcv_duty);
    x.set(C_MUX0, h.mux[0]);
    x.set(C_MUX1, h.mux[1]);
    x.set(C_MUX2, h.mux[2]);
    x.set(C_LOOP_US, (float)h.loop_us);
    x.set_u32(C_SEGMENTS, e.ne().segments());
    const Exhaust &ex = e.exhaust();
    x.set(C_EGT, ex.egt_c());
    x.set(C_EGT_V, ex.egt_v());
    x.set(C_LAMBDA, ex.lambda());
    x.set(C_WB_V, ex.wb_v());
    x.set(C_EGT_SCALE, ex.egt_scale() * 100);
    x.set(C_RICH_CUT, ex.rich_cut() * 100);
    x.set(C_SMOKE_TRIM, (ex.smoke_scale(s.map_kpa, e.ne().rpm()) - 1) * 100);
    x.set(C_Q_LIMIT, e.fuel().limit());
}

class Telemetry {
public:
    static constexpr uint32_t SAMPLE_PERIOD_US = 10000;    // 100 Hz
    static constexpr uint32_t SCHEMA_PERIOD_US = 1000000;  // every second

    Telemetry(LinkTx &tx, const char *fw) : tx_(tx), fw_(fw) {}

    // NE ISR, tooth 0, after Engine::on_ne(). Single producer.
    void on_segment(const Engine &e) {
        const NeDecoder &ne = e.ne();
        jlog::Injection r;
        r.t_us = ne.segment_start_us();
        r.period_us = ne.rpm_valid() ? (uint32_t)(30.0e6f / ne.rpm()) : 0;
        r.segment = ne.segments();
        const SpvPlan &p = e.spv_plan();
        r.q = e.commands().q;
        r.close_deg = p.close_deg;
        r.open_deg = p.open_deg;
        r.inject = p.inject;
        uint32_t h = head_, next = (h + 1) % QN;
        if (next == tail_) { inj_lost_++; return; }
        q_[h] = r;
        head_ = next;
    }

    // 1 kHz loop. Returns the number of bytes written.
    size_t poll(uint32_t now_us, const Engine &e, const Calibration &cal, const Sensors &s, const HalStatus &h) {
        size_t out = 0;
        if (!booted_) {
            booted_ = true;
            out += send_meta(cal);
            out += send_schema();
            out += event(now_us, jlog::E_BOOT, 0, 0);
            last_schema_ = last_sample_ = now_us;
            mode_ = (int)e.mode();
        }
        // Events on change.
        if ((int)e.mode() != mode_) {
            out += event(now_us, jlog::E_MODE, (uint32_t)(int)e.mode(), (uint32_t)mode_);
            mode_ = (int)e.mode();
        }
        uint32_t dtc = e.commands().dtc;
        if (dtc != dtc_) { out += event(now_us, jlog::E_DTC, dtc, dtc_); dtc_ = dtc; }
        uint32_t sl = e.ne().sync_losses();
        if (sl != sync_losses_) { out += event(now_us, jlog::E_SYNC_LOST, sl, 0); sync_losses_ = sl; }
        if (h.arm != arm_ || s.supervisor_ok != safe_ok_) {
            out += event(now_us, jlog::E_ARM, h.arm, s.supervisor_ok);
            arm_ = h.arm; safe_ok_ = s.supervisor_ok;
        }
        if (h.loop_us > 1000 && !overload_reported_) {
            out += event(now_us, jlog::E_OVERLOAD, h.loop_us, 0);
            overload_reported_ = true;
        }
        // Injection records queued by the ISR.
        while (tail_ != head_) {
            uint8_t p[jlog::Injection::SIZE];
            size_t n = q_[tail_].pack(p);
            tail_ = (tail_ + 1) % QN;
            out += send(jlog::T_INJ, p, n);
        }
        // Samples at 100 Hz, catching up at most one period.
        if (now_us - last_sample_ >= SAMPLE_PERIOD_US) {
            last_sample_ = (now_us - last_sample_ > 2 * SAMPLE_PERIOD_US) ? now_us : last_sample_ + SAMPLE_PERIOD_US;
            fill_sample(sample_, now_us, seq_++, e, cal, s, h);
            out += send(jlog::T_SAMPLE, sample_.data(), sample_.size());
        }
        if (now_us - last_schema_ >= SCHEMA_PERIOD_US) {
            last_schema_ = now_us;
            out += send_schema();
            out += send_meta(cal);
            uint32_t lost = dropped_ + inj_lost_;
            if (lost != reported_lost_) {
                out += event(now_us, jlog::E_DROPPED, lost - reported_lost_, inj_lost_);
                reported_lost_ = lost;
            }
            overload_reported_ = false;
        }
        return out;
    }

    uint32_t dropped() const { return dropped_ + inj_lost_; }
    uint32_t samples() const { return seq_; }

    // Send an event from outside (e.g. takeover denied).
    size_t event(uint32_t t_us, uint8_t code, uint32_t a, uint32_t b) {
        uint8_t p[13];
        return send(jlog::T_EVENT, p, jlog::event_payload(p, t_us, code, a, b));
    }

private:
    static constexpr uint32_t QN = 64;

    size_t send(uint8_t type, const uint8_t *p, size_t n) {
        uint8_t f[jlog::MAX_PAYLOAD + jlog::HEADER + jlog::TRAILER];
        size_t len = jlog::frame(f, sizeof f, type, p, n);
        if (!len || tx_.room() < len) { dropped_++; return 0; }
        tx_.write(f, len);
        return len;
    }
    size_t send_schema() {
        uint8_t p[jlog::MAX_PAYLOAD];
        return send(jlog::T_SCHEMA, p, jlog::schema_payload(p, sizeof p));
    }
    size_t send_meta(const Calibration &cal) {
        char t[200];
        int n = snprintf(t, sizeof t, "fw=%s\nformat=%d\nboard=junkr-ecu-revA\ncal_crc=%08lx\ncal_size=%u\n", fw_,
                         jlog::FORMAT_VERSION, (unsigned long)jlog::crc32(&cal, sizeof cal), (unsigned)sizeof cal);
        return send(jlog::T_META, (const uint8_t *)t, n > 0 ? (size_t)n : 0);
    }

    LinkTx &tx_;
    const char *fw_;
    jlog::Sample sample_;
    jlog::Injection q_[QN];
    volatile uint32_t head_ = 0, tail_ = 0, inj_lost_ = 0;
    uint32_t dropped_ = 0, reported_lost_ = 0, seq_ = 0;
    uint32_t last_sample_ = 0, last_schema_ = 0;
    uint32_t dtc_ = 0, sync_losses_ = 0;
    int mode_ = 0;
    bool booted_ = false, arm_ = false, safe_ok_ = true, overload_reported_ = false;
};

}  // namespace ecu
