// =====================================================================
//  ECU interposer — shared parameters
//
//  Header geometry (pitch, rows, cavity, ribs, rails) comes from the
//  TE 176122-6 model and lives in sections.scad. What is still an ESTIMATE
//  [MEASURE] is the receptacle/tab terminal body envelope and the plug
//  latch. Print fit_coupon.scad first with real terminals, then tune here.
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

// Retention. The 176122-6 model shows no lock ramp, so the OEM lock geometry is
// unknown [MEASURE on the truck]. The printed plug relies on its key ridges plus
// a zip-tie saddle; the shroud gets a window the OEM lock arm can click into
// once its position is known.
ziptie_w       = 4.0;
lock_window    = false;
lock_window_x  = 0;     // from section centre
lock_window_z  = 3.0;   // from the mouth
lock_window_w  = 2.4;
lock_window_l  = 2.0;

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
