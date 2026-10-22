// =====================================================================
//  Fit coupon — print this FIRST (~10 min on resin).
//
//  Small female + male blocks for each terminal family. Use real
//  terminals to check: tab entry, body slide-in, lance click, pull-out
//  force, and pitch against the OEM plug. Adjust params.scad
//  (clr, T040/T070 body sizes, pitch) until it feels like the OEM part.
// =====================================================================

include <interposer.scad>

COUPON_040 = [ "040", 3, 2, 2.5, 3.6, [[1,1,1],[1,1,1]], [] ];
COUPON_070 = [ "070", 2, 1, 5.0, 3.6, [[2,2]], [] ];

translate([-12, 0, 0]) female_plug(COUPON_040, with_flange = false);
translate([ 12, 0, 0]) female_plug(COUPON_070, with_flange = false);
translate([-12, 22, 0]) male_shroud(COUPON_040, with_flange = false);
translate([ 12, 22, 0]) male_shroud(COUPON_070, with_flange = false);
