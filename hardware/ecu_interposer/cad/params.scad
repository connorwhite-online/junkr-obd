// =====================================================================
//  ECU interposer — shared parameters
//
//  Header geometry (pitch, rows, cavity, ribs, rails) comes from the
//  TE 176122-6 model and lives in sections.scad. What is still an ESTIMATE
//  [MEASURE] is the receptacle/tab terminal body envelope. The plug latch
//  comes from TE's 175110-6 STEP model. Print fit_coupon.scad first with
//  real terminals, then tune here.
//  All units mm.
// =====================================================================

$fn = 32;

// ---- Printer / process -------------------------------------------------
clr        = 0.15;   // per-side clearance on terminal cavities (resin: 0.10, FDM: 0.20)
slide_clr  = 0.25;   // per-side clearance for sliding fits (plug-in-shroud, tenons)
min_wall   = 0.6;    // thinnest wall the process can hold (resin 0.5, 0.4-nozzle FDM 0.8)

// ---- Terminal families ---------------------------------------------------
// The 176122-6 hybrid header carries two TE Multilock families:
//   040: 1.0 x 0.64 mm tab, 3.0 mm pitch here  (receptacle 175061-1/175062-1/173681-6, tab 173682-1)
//   070: 1.8 x 0.64 mm tab, 3.5 mm pitch here  (receptacle 173631-1/-8, tab 173645-1)
// Tab sizes are catalogue values. Body envelopes and lance positions are
// [MEASURE]: TE does not publish them openly.
//
// index:            0        1       2        3       4         5          6          7
//                  tab_w   tab_t   body_w   body_h  body_len  lance_h   lance_from  male_body_len
//                                                             (spring)  _front      (behind the floor)
T040 = [            1.00,   0.64,   2.00,    2.20,   16.0,     0.45,     5.0,        8.0 ];
T070 = [            1.80,   0.64,   2.85,    2.80,   16.0,     0.55,     6.0,        9.0 ];

function tab_w(t)      = t[0];
function tab_t(t)      = t[1];
function body_w(t)     = t[2];
function body_h(t)     = t[3];
function body_len(t)   = t[4];
function lance_h(t)    = t[5];
function lance_ff(t)   = t[6];
function male_body_len(t) = t[7];

function term(code) = code == 2 ? T070 : T040;

// ---- Housing geometry ----------------------------------------------------
face_wall      = 1.0;   // front wall of the female plug (tab entry holes go through it)
plug_tail      = 6.0;   // female plug length behind the header mouth (finger grip + terminal length)
shroud_floor   = 9.0;   // male shroud back wall that holds the tab terminals
key_clr        = 0.20;  // per-side clearance of the plug's key ridges in the header slots

// Retention: the OEM lock, measured from TE's 175110-6 STEP model (see
// docs/ECU_INTERPOSER_RESEARCH.md §2.4). All three plugs share one latch on
// the section centre line, between the two slots either side of the 2 mm
// centre rib. The plug has two flexible tines (|x| = 1.3..4.1) rooted at its
// front, each with a 45° hook whose lock face sits 11.45 mm behind the plug
// front. The header holds a lock bar at its mouth between the tines; the
// hooks ride under it and snap up behind it.
//
// Lock bar inside the male shroud (the OEM plug clicks onto it):
lock_bar       = true;
lock_bar_hw    = 2.55;   // half width; the OEM tines' outer parts start at |x| = 2.8
lock_bar_y0    = 13.91;  // underside, cavity frame (OEM arm top 13.61, hook top 14.86)
lock_bar_depth = 2.9;    // mouth to the bar's inner face (OEM lock face: 3.05..3.55 in)
// Lock tines on the printed female plug (they click onto the ECU header's bar).
// Kept below 13.45 everywhere the real bar could be, hook up to 14.8.
plug_latch     = true;
latch_x        = [1.4, 2.6];  // tine span either side of the centre line
latch_top      = 13.45;  // tine top (cavity frame)
latch_t        = 1.0;    // tine thickness
latch_root     = 1.5;    // tines are solid to the body for z < latch_root
latch_hook_top = 14.8;   // 0.22 under the header slot ceiling
latch_lock_z   = 11.1;   // hook lock face from the plug front = 3.2 inside the mouth (OEM: 3.05)
latch_ramp     = 1.35;   // 45° lead ramp
latch_flat     = 0.45;
latch_end      = 18.0;   // free end: crossbar + press tab (outside the header mouth)
latch_tab_l    = 2.5;
latch_pocket   = 0.6;    // body relief under the tines so they can deflect 1.4 mm
latch_guard_x  = 3.0;    // key ridges in the centre slots start at |x| = this
ziptie_w       = 4.0;    // backup tie to the ECU bracket

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
