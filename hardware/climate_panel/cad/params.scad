// =====================================================================
//  Climate panel replacement: shared parameters
//
//  A printed faceplate that replaces the front of the stock climate
//  control panel: one big temperature knob (an M5Stack Dial, which is a
//  knob, a round screen and an ESP32-S3 in one) and key-switch buttons for
//  everything else. See docs/CLIMATE_PANEL.md for why, and for what each
//  button is wired to.
//
//  Frame: X across the dash (+ = towards the passenger door on this RHD
//  truck), Y up, Z towards the driver. The visible face is at Z = 0, so
//  everything behind it has negative Z. All units mm.
//
//  Anything marked [MEASURE] is a guess until Connor measures the truck.
//  The guide (build guide step 1) lists what to measure and how.
// =====================================================================

$fn = 64;

// ---- Process ------------------------------------------------------------
fit_clr   = 0.30;   // per-side clearance: skirt inside the dash opening
cap_clr   = 0.40;   // per-side gap: key cap in its face opening
dial_clr  = 0.60;   // per-side gap: the Dial's turning ring in the face hole
mx_clr    = 0.00;   // MX plate hole is 14.0 nominal; add 0.05-0.1 for FDM

// ---- Dash opening (the hole the stock panel leaves) [MEASURE] -------------
open_w    = 176;    // [MEASURE] width of the opening
open_h    = 58;     // [MEASURE] height of the opening
open_r    = 3;      // [MEASURE] corner radius of the opening
open_tilt = 0;      // [MEASURE] how far the opening leans back, degrees (info only)

// ---- Faceplate ------------------------------------------------------------
overlap   = 3.0;    // face overhangs the opening by this much all round
face_w    = open_w + 2 * overlap;
face_h    = open_h + 2 * overlap;
face_r    = open_r + overlap;
face_t    = 3.0;    // face thickness
face_chamfer = 1.0; // front edge break

skirt_wall = 2.0;   // the sleeve that goes into the dash opening
skirt_d    = 30;    // [MEASURE] depth behind the face; free depth must be more

// Mounting ears on the skirt sides, where the stock panel's screws are.
// [x, y] of each screw hole, in the face frame; ear_z is the depth of the
// ear behind the face (the dash wall thickness plus a little).
ears      = [[-97, 14], [-97, -14], [97, 14], [97, -14]];  // [MEASURE]
ear_z     = 8;      // [MEASURE] depth of the ear face behind the visible face
ear_t     = 2.5;    // ear thickness
ear_hole  = 4.5;    // clearance for the stock screw (M4 or a 4.2 self-tapper) [MEASURE]

// ---- Key switches (Cherry MX footprint: Gateron, Kailh, Akko all fit) -----
mx_hole   = 14.0 + 2 * mx_clr;  // plate cut-out
mx_plate  = 1.5;    // plate thickness the clips expect
mx_above  = 11.6;   // plate top to stem top, switch at rest
mx_housing= 6.6;    // plate top to housing top
mx_travel = 4.0;
mx_below  = 8.3;    // plate top to pin tips

cap_s     = 15.5;   // square cap, smaller than a keyboard's 18 so two rows fit
cap_r     = 2.0;
cap_h     = 7.0;    // cap body height (stem socket hangs below this)
cap_proud = 2.0;    // cap top sits this far in front of the face at rest
legend_d  = 0.6;    // legend engrave depth
legend_size = 3.2;

// Plate top position follows from the cap: cap underside = stem top.
plate_z   = cap_proud - cap_h - mx_above;   // = -16.6 with the defaults

// Buttons: [label, x, y]. Labels are engraved on the caps; "" = blank cap.
// The default layout keeps every stock button except TEMP up/down, which
// the knob replaces. Move, add or drop entries freely: the face, the plate
// and the caps all follow this list.
pitch = 19.0;
buttons = [
    ["AUTO",   -78,  pitch/2], ["A/C",   -59,  pitch/2], ["MODE", -40,  pitch/2],
    ["OFF",    -78, -pitch/2], ["REC",   -59, -pitch/2], ["DEF",  -40, -pitch/2],
    ["FAN+",    40,  pitch/2], ["R.DEF",  59,  pitch/2],
    ["FAN-",    40, -pitch/2], ["",       59, -pitch/2],
];

// Plate standoffs: [x, y]. Keep them clear of switch bodies (14 mm square
// around each button) and the Dial cup.
posts     = [[-49.5, 0], [49.5, 0], [-30, 23], [30, 23], [-30, -23], [30, -23]];
post_d    = 5.0;
post_pilot= 2.2;    // M2.5 self-tapping (or a 3.2 hole for an M2.5 heat-set)
post_screw= 2.8;    // clearance in the plate

// ---- The knob: M5Stack Dial v1.1 (SKU K130) -------------------------------
// Published: 51.0 x 51.0 x 32.3 mm. Which part turns, and how far the turning
// ring reaches down the body, is [VERIFY] on the real unit.
dial_d     = 51.0;  // [VERIFY]
dial_h     = 32.3;
dial_x     = 0;
dial_y     = 0;
dial_proud = 10;    // how far the knob stands in front of the face
dial_grip_from = 12;// [VERIFY] the base that does not turn reaches this far up from the back
cup_wall   = 2.0;
cup_floor  = 1.5;
rib_h      = 0.45;  // six crush ribs grip the fixed base (FDM: 0.35-0.5)
dial_back_screws = []; // [VERIFY] [[x, y], ...] M2 holes in the Dial's back shell, if usable
cable_slot = [14, 8]; // USB-C and the Grove leads leave through the cup floor

// Optional grip sleeve over the turning ring, for a fatter knob.
sleeve     = false;
sleeve_od  = 58;
sleeve_h   = 8;

// ---- Rear cover -------------------------------------------------------------
cover_t    = 2.0;
grommet    = [36, 12];   // harness pass-through (stock plug leads, power)
perf_holes = [[-20, -12], [20, -12], [-20, 12], [20, 12]]; // 50x30 perfboard [VERIFY]
