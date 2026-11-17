// Calibration for the 1KZ-TE (KZJ95, A343F). Every number here is a
// starting point, not a measured Toyota value. The shadow-mode learner
// (shadow.h) replaces the fuel and timing maps with what the stock ECU
// actually does, before this ECU is ever allowed to take over. [VERIFY]
#pragma once
#include <stdint.h>
#include "table.h"

namespace ecu {

// What is wired to each AUX analog input (J7 pins 7 and 8, 0-5 V).
enum class AuxUse : uint8_t { None, Egt, Wideband };

struct Calibration {
    // Driver request: Q (mm3/stroke) from pedal % (x) and rpm (y).
    Table2D<6, 6> q_driver{
        {0, 10, 25, 50, 75, 100},
        {600, 1000, 1800, 2600, 3400, 4200},
        {{6, 14, 26, 42, 56, 64},
         {5, 13, 25, 42, 57, 66},
         {4, 12, 24, 42, 58, 66},
         {3, 11, 23, 41, 56, 64},
         {2, 10, 22, 40, 53, 60},
         {0, 8, 20, 36, 48, 54}}};
    // Full-load limit (torque curve) by rpm.
    Table1D<6> q_max{{600, 1200, 2000, 2800, 3600, 4400}, {45, 58, 66, 66, 60, 50}};
    // Smoke limit by boost (MAP kPa abs, x) and rpm (y).
    Table2D<5, 3> q_smoke{
        {100, 130, 160, 190, 220},
        {1000, 2500, 4000},
        {{40, 50, 60, 66, 66},
         {38, 48, 58, 66, 66},
         {36, 46, 56, 64, 66}}};
    // Cranking Q by coolant temp (deg C).
    Table1D<4> q_start{{-20, 0, 40, 80}, {70, 60, 45, 35}};
    float idle_rpm_warm = 700, idle_rpm_cold = 950;  // target, by coolant 80 / -20 C
    float idle_kp = 0.012f, idle_ki = 0.010f;          // mm3 per rpm, mm3 per rpm.s
    float idle_q_trim_max = 8;

    // Spill: Q delivered (z, mm3/st) for SPV opening angle (x, deg CA after
    // NE tooth 0) at rpm (y). The plunger starts lifting near tooth 2.
    Table2D<7, 4> spill{
        {20, 30, 40, 50, 60, 70, 80},
        {500, 1500, 3000, 4500},
        {{0, 12, 26, 40, 54, 66, 76},
         {0, 11, 25, 39, 53, 65, 75},
         {0, 10, 23, 37, 51, 63, 73},
         {0, 9, 21, 35, 49, 61, 71}}};
    float spv_close_deg = 5.0f;     // energize (close) the SPV, before lift starts
    float spv_open_delay_us = 300;  // SPV response time, compensated in the schedule [VERIFY]

    // Target advance (deg BTDC) by Q (x) and rpm (y).
    Table2D<5, 5> adv_target{
        {0, 15, 30, 45, 66},
        {600, 1500, 2500, 3500, 4500},
        {{2, 3, 4, 5, 5},
         {4, 5, 6, 7, 7},
         {7, 8, 9, 10, 10},
         {10, 11, 12, 13, 13},
         {12, 13, 14, 15, 15}}};
    Table1D<4> adv_ect_add{{-20, 0, 40, 80}, {4, 3, 1, 0}};
    float tdc_cal_offset = 30.0f;   // advance = TDC phase in segment - offset [VERIFY]
    float tcv_kp = 4.0f, tcv_ki = 6.0f, tcv_ff = 35.0f;  // duty % per deg, % per deg.s, base %
    float tcv_fixed_duty = 30.0f;   // cranking / stall / no TDC (OEM does the same)

    // Limits
    float rpm_soft_cut = 4600, rpm_hard_cut = 4900;  // fuel taper / zero fuel
    float rpm_svr_cut = 5200;                         // supervisor drops the SVR relay
    float rpm_crank_exit = 500;
    float pedal_mismatch_pct = 12;                    // VA above this with IDL closed = fault

    // ---- Exhaust sensors on the AUX plug (exhaust.h, docs/standalone-ecu/EXHAUST.md)
    AuxUse aux_use[2] = {AuxUse::None, AuxUse::None};   // AUX_AN1, AUX_AN2
    // EGT amplifier: two points of its linear 0-5 V output. [VERIFY] from its datasheet.
    float egt_v0 = 0.0f, egt_c0 = 0, egt_v1 = 5.0f, egt_c1 = 1250;
    // Pre-turbo EGT protection: the fuel limit falls linearly from 100 % at
    // start to egt_min_scale at end. [VERIFY] 1KZ-TE: aluminium head, keep it cool.
    float egt_derate_start_c = 650, egt_derate_end_c = 750, egt_min_scale = 0.6f;
    float egt_fault_scale = 0.85f;   // fuel limit while a fitted EGT sensor has failed
    // Wideband controller: two points of its linear analog output. [VERIFY]
    // against the controller's output setting.
    float wb_v0 = 0.0f, wb_l0 = 0.5f, wb_v1 = 5.0f, wb_l1 = 2.5f;
    float wb_warmup_s = 25;          // ignore the sensor until the engine has run this long
    // Smoke target: leanest-allowed lambda at the fuel limit, by rpm. [VERIFY] on the road.
    Table1D<4> lambda_target{{1000, 2000, 3000, 4000}, {1.30f, 1.25f, 1.22f, 1.25f}};
    float lambda_floor = 1.10f;      // richer than this anywhere: pull fuel at once
    // Live smoke-limit trim, stored per q_smoke cell. Fuel can come off much
    // further than it can go on, and it only goes on with a healthy EGT.
    float trim_min = -0.15f, trim_max = 0.05f;
    float trim_rate_rich = 0.25f;    // trim per second per unit lambda error, too rich
    float trim_rate_lean = 0.04f;    // the same, too lean (slow on purpose)
    float rich_cut_rate = 1.5f, rich_cut_max = 0.3f;   // floor protection, per s and max fraction

    // A343F shift schedule, vehicle km/h against pedal % (x).
    Table1D<4> up12{{0, 30, 60, 100}, {14, 20, 30, 45}};
    Table1D<4> up23{{0, 30, 60, 100}, {30, 40, 58, 85}};
    Table1D<4> up3o{{0, 30, 60, 100}, {48, 58, 80, 120}};
    float down_hyst_kmh = 8;
    float lockup_on_kmh = 62, lockup_off_kmh = 55, lockup_max_pedal = 60;
};

}  // namespace ecu
