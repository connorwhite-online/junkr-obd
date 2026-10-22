// =====================================================================
//  Connector sections: TE/AMP 176122-6 "hybrid cap assy 64P"
//  (the ECU-side header on the 1KZ-TE Denso ECU), split 26 / 16 / 22.
//
//  Source: rusEFI's 176122-6 KiCad footprint + 3D model (rusefi-hardware,
//  rusefi/kicad-libraries), measured from the mesh (±0.05 mm), and
//  cross-checked against TE's own data (5.2 mm row spacing on 175110-6).
//  See docs/ECU_INTERPOSER_RESEARCH.md §2.
//
//  Frame (cavity-relative): X along the row from the cavity's left inner
//  wall, Y up from the cavity floor, Z = mating axis.
//
//  Mating plugs (TE): 22P 175110-6, 16P 175109-6, 26P 175111-x (inferred).
//
//  Names follow the 1996 KZJ95 wiring manual (EWD269F p.210): E5 = 26 pin
//  ("A"), E6 = 16 pin ("B"), E8 = 22 pin ("C"). Older KZN130 tables use the
//  letters the other way round (A = 22, C = 26), so go by the pin count.
//
//  Pin numbers: the manual draws each plug from its mating face, which is
//  the mirror image of this frame (header seen from its mouth). So manual
//  column 1 is the rightmost column here. Top row = upper row (y = 7.05/8.05),
//  numbered 1..n left to right in the manual; bottom row continues n+1..2n.
//  section = [ name, cavity_len, cols, top_slots, rail ]
//    cols      = list of [ x, type ]    type 1 = 040 (1.0 mm tab), 2 = 070 (1.8 mm tab)
//                (every column carries two pins, upper and lower row)
//    top_slots = list of [ x0, x1 ]     recesses in the top wall (the plug's key ridges ride in them)
//    rail      = [ y0, y1, protrude ]   polarising rail on both end walls
// =====================================================================

// Shared header geometry (mm)
HDR_CAV_H      = 11.82;   // floor to rib undersides
HDR_SLOT_TOP   = 15.02;   // floor to top of the slots between ribs
HDR_DEPTH      = 14.50;   // cavity depth, mouth to floor
HDR_WALL       = 1.48;
ROW_PITCH      = 5.20;
ROW_Y_040      = [7.05, 1.85];   // upper, lower row centres above the floor
ROW_Y_070      = [8.05, 2.85];   // 070 rows sit 1.0 higher
TAB_PROUD_040  = 7.09;           // tab tip above the floor
TAB_PROUD_070  = 7.99;

function s_name(s)  = s[0];
function s_len(s)   = s[1];
function s_cols(s)  = s[2];
function s_slots(s) = s[3];
function s_rail(s)  = s[4];
function n_pins(s)  = 2 * len(s_cols(s));

// Toyota pin number of column i (0 = leftmost in this frame), row r (0 = upper)
function pin_no(s, i, r) = (len(s_cols(s)) - i) + r * len(s_cols(s));

function seq(x0, pitch, n, t) = [ for (i = [0 : n - 1]) [ x0 + i * pitch, t ] ];

// E5 — 26 pin: 070 x3 | 040 x8 | 070 x2 (in this frame). Pump/crank sensors,
// SPV/TCV drives, A/T solenoids, grounds.
SEC_E5 = [ "E5_26P", 42.44,
  concat(seq(2.37, 3.5, 3, 2), seq(12.47, 3.0, 8, 1), seq(36.57, 3.5, 2, 2)),
  [ [1.52, 3.24], [7.66, 9.38], [15.20, 20.24], [22.22, 27.22], [33.06, 34.78], [39.22, 40.92] ],
  [ 5.26, 5.94, 0.80 ] ];

// E6 — 16 pin: 040 x8. Analog sensors and diagnostics.
SEC_E6 = [ "E6_16P", 25.20,
  seq(2.10, 3.0, 8, 1),
  [ [1.48, 3.22], [6.60, 11.60], [13.58, 18.60], [21.98, 23.70] ],
  [ 6.12, 7.08, 0.50 ] ];

// E8 — 22 pin: 070 x1 | 040 x8 | 070 x2 (in this frame). Power, relays, switches.
SEC_E8 = [ "E8_22P", 35.42,
  concat(seq(2.37, 3.5, 1, 2), seq(5.47, 3.0, 8, 1), seq(29.57, 3.5, 2, 2)),
  [ [1.50, 3.24], [5.86, 7.58], [11.70, 16.72], [18.72, 23.72], [27.86, 29.58], [32.20, 33.92] ],
  [ 5.26, 5.94, 0.80 ] ];

// Fit coupon: one 070 / two 040 / one 070, real pitch and row spacing, no keys.
SEC_COUPON = [ "FIT", 13.94,
  [ [2.37, 2], [5.47, 1], [8.47, 1], [11.57, 2] ],
  [ ],
  [ 5.26, 5.94, 0.80 ] ];

SECTIONS = [ SEC_E5, SEC_E6, SEC_E8 ];   // sec_i 0, 1, 2
