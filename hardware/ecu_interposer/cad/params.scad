// =====================================================================
//  ECU interposer — shared parameters
//
//  EVERY dimension marked [MEASURE] is an estimate from TE/AMP Multilock
//  catalogue data, not from the actual KZJ95 connector. Print
//  fit_coupon.scad first, compare against the real plug, then edit here.
//  All units mm.
// =====================================================================

$fn = 32;

// ---- Printer / process -------------------------------------------------
clr        = 0.15;   // per-side clearance on terminal cavities (resin: 0.10, FDM: 0.20)
slide_clr  = 0.25;   // per-side clearance for sliding fits (plug-in-shroud, tenons)
min_wall   = 0.6;    // thinnest wall the process can hold (resin 0.5, 0.4-nozzle FDM 0.8)

// ---- Terminal families ---------------------------------------------------
// Toyota mid-90s Denso ECUs use TE/AMP "Multilock" hybrid headers:
//   040 series: 1.0 x 0.64 mm tab, 2.5 mm pitch  (signal)
//   070 series: 1.8 x 0.64 mm tab                (power / solenoid drive)
// Receptacle body sizes below are [MEASURE] — catalogue tab sizes are firm,
// the stamped body envelope is not published.
//
// index:            0        1       2        3       4         5          6          7
//                  tab_w   tab_t   body_w   body_h  body_len  lance_h   lance_from  tab_len
//                                                             (spring)  _front      (male, exposed)
T040 = [            1.00,   0.64,   1.90,    2.10,   14.2,     0.45,     5.0,        6.5 ];
T070 = [            1.80,   0.64,   2.80,    2.70,   16.0,     0.55,     6.0,        7.5 ];

function tab_w(t)      = t[0];
function tab_t(t)      = t[1];
function body_w(t)     = t[2];
function body_h(t)     = t[3];
function body_len(t)   = t[4];
function lance_h(t)    = t[5];
function lance_ff(t)   = t[6];
function tab_len(t)    = t[7];

function term(code) = code == 2 ? T070 : T040;

// ---- Housing geometry ----------------------------------------------------
face_wall      = 1.0;   // front wall of female plug (tab entry holes go through it)
shell_wall     = 1.6;   // outer wall of plugs / shrouds
plug_len_extra = 2.0;   // female plug length beyond terminal body length
shroud_depth   = 11.0;  // [MEASURE] how deep the OEM plug sits in the ECU header
shroud_floor   = 6.0;   // floor thickness that holds male tabs (needs > lance step)

// Latch (cantilever on the plug, ramp in the shroud) — [MEASURE] all
latch_w        = 5.0;
latch_len      = 9.0;
latch_t        = 1.2;
latch_hook     = 0.9;   // hook height
latch_gap      = 0.6;   // flex gap under the arm

// Chamber between the two ends (wire pass-through + tap pick-off)
chamber_len    = 26.0;  // room for pigtail loops + tap splices
chamber_wall   = 1.8;
tap_slot_w     = 14.0;  // exit slot for tap ribbon / JST-SH cable
tap_slot_h     = 4.0;
screw_d        = 2.2;   // M2 self-tapper pilot
screw_head_d   = 4.2;

// Tap-board bay (separate enclosure, ribbon-linked to each section)
tapbox_pcb     = [60, 40, 1.6];   // custom tap PCB (see docs/ECU_TAP_ARCHITECTURE.md)
tapbox_parts_h = 11.0;            // tallest component above PCB (ESP32-S3 module + shield can)
tapbox_wall    = 2.0;
tapbox_inlets  = 3;               // one ribbon inlet per connector section
