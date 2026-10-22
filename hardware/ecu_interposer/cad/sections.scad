// =====================================================================
//  Connector section definitions — 1KZ-TE engine ECU (KZJ90/95, KZN130/165/185)
//
//  The 1KZ-TE Denso ECU uses three harness plugs: A (22), B (16), C (26)
//  = 64 pins, which matches the TE/AMP 176122-6 64-way hybrid header used
//  on several mid-90s Toyota ECUs. Pin numbering below is row-major,
//  row 0 = pins 1..n/2 (top row, latch side), viewed from the WIRE side of
//  the OEM harness plug.  [MEASURE] against the real plug before printing.
//
//  Pin codes:  0 = no cavity   1 = 040 Multilock   2 = 070 Multilock
//
//  section = [ name, cols, rows, pitch_x, pitch_y, pinmap, keys ]
//    keys = list of [ x_from_left_edge, width, face ]   face: 0 top, 1 bottom
// =====================================================================

function s_name(s)  = s[0];
function s_cols(s)  = s[1];
function s_rows(s)  = s[2];
function s_px(s)    = s[3];
function s_py(s)    = s[4];
function s_map(s)   = s[5];
function s_keys(s)  = s[6];

// Plug A — 22 pin. Power / switched inputs (+B, BATT, IGSW, STA, A/C, SP1, TAC...)
SEC_A = [ "A22", 11, 2, 2.5, 3.6,
  [ [1,1,1,1,1,1,1,1,1,1,1],
    [1,1,1,1,1,1,1,1,1,1,1] ],
  [ [3.0, 1.6, 0] ] ];

// Plug B — 16 pin. Analog sensors (VA, PIM, THA, THW, THF, VRP, VRT, VC, E2...)
SEC_B = [ "B16", 8, 2, 2.5, 3.6,
  [ [1,1,1,1,1,1,1,1],
    [1,1,1,1,1,1,1,1] ],
  [ [6.0, 1.6, 0] ] ];

// Plug C — 26 pin. Pump/crank signals and actuator drives
// (NE+/-, TDC+/-, SPV, TCV, SVR, EGR, S/TH, E01/E02 ...)
SEC_C = [ "C26", 13, 2, 2.5, 3.6,
  [ [1,1,1,1,1,1,1,1,1,1,1,1,1],
    [1,1,1,1,1,1,1,1,1,1,1,1,1] ],
  [ [3.0, 1.6, 1], [20.0, 1.6, 0] ] ];

SECTIONS = [ SEC_A, SEC_B, SEC_C ];
