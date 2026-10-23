// =====================================================================
//  Fit coupon: print this FIRST.
//
//  A 4-column slice of the real 176122-6 header geometry: one 070 column,
//  two 040 columns, one 070 column, at the true pitch and row spacing,
//  without the per-section keys. Check against the truck:
//    - the female block slides onto the ECU header (wall/rail fit)
//    - the male block accepts the OEM harness plug's terminals
//    - real terminals (TE 175061-1 / 173631-1) click in and hold
//  Tune clr / slide_clr / T040 / T070 in params.scad until it feels OEM.
// =====================================================================

include <interposer.scad>

translate([0, 0, 0])  female_plug(SEC_COUPON, with_flange = false);
translate([0, 30, 0]) male_shroud(SEC_COUPON, with_flange = false);
