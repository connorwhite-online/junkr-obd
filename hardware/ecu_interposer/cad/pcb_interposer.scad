// =====================================================================
//  PCB interposer (rev C): one board between the ECU and all three
//  harness plugs (E5, E6, E8), with an ESP32-S3-DevKitC-1 on it, plus a
//  printed tray + lid.
//
//    harness plugs ==> TE 176122-6 header (real part) ==board== OEM-style
//                      female plugs on 20 mm tails ==> ECU
//
//  Board: kicad/interposer/ (KiCad 7). Board data for this file comes
//  from kicad/interposer/export_scene.py -> pcb_board_data.scad.
//
//  Frame: X = footprint x, Y toward the harness (+) / ECU (-), Z up from
//  the board's bottom face. Header mouth (harness edge) at Y = +25.5.
//
//  openscad -o ../stl/pcb_tray.stl -D 'part="tray"' pcb_interposer.scad
//  openscad -o ../stl/pcb_lid.stl  -D 'part="lid"'  pcb_interposer.scad
//  part: "assembly" | "exploded" | "tray" | "lid" | "board"
// =====================================================================

include <interposer.scad>
include <pcb_board_data.scad>

part = "assembly";

// ---- placement --------------------------------------------------------
SECS     = [SEC_E5, SEC_E6, SEC_E8];
SEC_XC   = [-27.104, -62.401, -94.188];   // cavity centres in footprint x
ON       = [0, 1, 2];                     // all three plugs go through the board
HDR_CUT  = 0.0;                           // the 176122-6 is used whole
DK_X     = -138.5;                        // DevKitC centre x; pin 1 (antenna end) at Y = -14
DK_PIN1  = -14.0;
LID_X0   = -119.9;                        // the lid covers the tails only, not the DevKitC
ZF       = PCB_T + HDR_WALL + HDR_GROOVE_D;   // header cavity floor above the board bottom
HDR_MOUTH = 25.5;                         // harness edge of the header
HDR_BACK  = 7.0;                          // back face of the header body
PLUG_REAR = -8.5;                         // rear face of the ECU-side plugs
OEM_TAIL  = 11.0;                         // OEM plug: wire-exit block behind the printed-plug length
function plug_total(s) = plug_len(s) + OEM_TAIL;

// [VERIFY] the real header's height above the board; this copies the
// cavity geometry from sections.scad (body ~19 mm tall).

module sec_T(i, yo) {
    translate([SEC_XC[i], yo, ZF]) mirror([1, 0, 0]) mirror([0, 1, 0]) rotate([90, 0, 0]) children();
}

// ---- parts that are bought -------------------------------------------
module header_176122() {
    color("#3a3a3a") intersection() {
        translate([-200, -100, -10]) cube([200 + HDR_CUT, 200, 100]);
        union() { for (i = ON) {
            s = SECS[i];
            sec_T(i, HDR_MOUTH - sh_len(s))
                intersection() {
                    male_shroud(s, false);
                    translate([-100, -50, sh_len(s) - (HDR_MOUTH - HDR_BACK)]) cube([200, 100, 100]);
                }
        }
        // board-lock ears at both ends
        translate([-117.8, HDR_BACK, PCB_T]) cube([4.4, 8.5, 5.5]);
        translate([-4.4, HDR_BACK, PCB_T]) cube([4.4, 8.5, 5.5]);
    } }
    // right-angle pins: out of the back face, bend down into the board
    color("#c8a24a")
        for (i = ON) {
            s = SECS[i];
            for (c = s_cols(s), r = [0, 1]) {
                x = SEC_XC[i] - cx(s, c[0]);
                z = ZF + row_y(c[1], r);
                yb = r == 0 ? 0.0 : 4.5;
                w = c[1] == 2 ? 1.8 : 1.0;
                translate([x - w / 2, yb - 0.32, z - 0.32]) cube([w, HDR_BACK - yb + 0.32, 0.64]);
                translate([x - 0.32, yb - 0.32, -1.2]) cube([0.64, 0.64, z + 0.32 + 1.2]);
            }
        }
}

module oem_plug(i) {
    s = SECS[i];
    female_plug(s, false);
    translate([0, 0, plug_len(s) - 0.01]) plug_envelope(s, 0, OEM_TAIL);
}

module ecu_plugs() {
    for (i = ON) color("#d9d4c7") sec_T(i, PLUG_REAR - plug_total(SECS[i])) oem_plug(i);
}

module harness_plugs(gap = 0) {
    for (i = ON) color("#5d6066") sec_T(i, HDR_MOUTH - insert_len(SECS[i]) + gap) oem_plug(i);
}

// short tails: plug rear -> straight back -> down into the board
module tails() {
    for (t = PCB_TAILS) {
        xh = t[0]; yh = t[1]; xc = t[2]; ty = t[3]; r = t[4];
        z = ZF + row_y(ty, r);
        d = ty == 2 ? 1.6 : 1.2;
        color(r == 0 ? "#b5452f" : "#2f6fb5") {
            hull() { translate([xc, PLUG_REAR, z]) sphere(d = d, $fn = 8); translate([xh, yh, z]) sphere(d = d, $fn = 8); }
            hull() { translate([xh, yh, z]) sphere(d = d, $fn = 8); translate([xh, yh, PCB_T]) sphere(d = d, $fn = 8); }
        }
    }
}

module board2d() {
    o = PCB_OUTLINE; c = PCB_CUT;
    difference() {
        translate([o[0], o[1]]) square([o[2] - o[0], o[3] - o[1]]);
        translate([c[0], c[1]]) square([c[2] - c[0] + 1, c[3] - c[1] + 1]);
    }
}

module pcb_board() {
    color("#1f5d3a") difference() {
        linear_extrude(PCB_T) board2d();
        for (h = PCB_HOLES) translate([h[0], h[1], -1]) cylinder(d = h[2], h = PCB_T + 2, $fn = 12);
    }
    color("#d4a94a") for (h = PCB_HOLES) if (h[4] == 1)
        for (z = [-0.03, PCB_T]) translate([h[0], h[1], z]) difference() {
            cylinder(d = h[3], h = 0.03, $fn = 16);
            translate([0, 0, -1]) cylinder(d = h[2], h = 2, $fn = 12);
        }
    color("#b87333") for (t = PCB_TRACKS)
        let (z = t[5] == 0 ? PCB_T : -0.035)
            hull() {
                translate([t[0], t[1], z]) cylinder(d = t[4], h = 0.035, $fn = 8);
                translate([t[2], t[3], z]) cylinder(d = t[4], h = 0.035, $fn = 8);
            }
    // bottom-side resistors / caps: one body per pad pair (approximate)
    color("#202020") for (p = PCB_SMD) if (p[4] == 1)
        translate([p[0] - p[2] / 2, p[1] - p[3] / 2, -0.55]) cube([p[2], p[3], 0.5]);
}

module devkitc() {                   // ESP32-S3-DevKitC-1 on 1x22 headers, USB at +Y
    z = PCB_T + 2.5;
    y0 = DK_PIN1 - 8.0; y1 = DK_PIN1 + 21 * 2.54 + 7.0;
    color("#151515") for (r = [-1, 1]) translate([DK_X + r * 11.43 - 1.27, DK_PIN1 - 1.27, PCB_T]) cube([2.54, 22 * 2.54, 2.5]);
    color("#20232a") translate([DK_X - 12.7, y0, z]) cube([25.4, y1 - y0, 1.6]);
    color("#c0c0c0") translate([DK_X - 9.0, y0 + 1.0, z + 1.6]) cube([18.0, 15.0, 3.1]);       // WROOM module can
    color("#2a5db0") translate([DK_X - 9.0, y0 + 0.5, z + 1.6]) cube([18.0, 6.0, 0.8]);        // antenna
    color("#c0c0c0") for (dx = [-6.5, 6.5]) translate([DK_X + dx - 4.5, y1 - 7.0, z + 1.6]) cube([9.0, 7.5, 3.2]);  // USB-C x2
    color("#c8a24a") for (r = [-1, 1], k = [0 : 21])
        translate([DK_X + r * 11.43 - 0.32, DK_PIN1 + k * 2.54 - 0.32, -1.2]) cube([0.64, 0.64, z + 1.6 + 1.2 + 1.0]);
}

// ---- printed parts ----------------------------------------------------
wall   = 1.4;
floor_t = 1.2;
gap_b  = 2.0;                        // board bottom to tray floor (leads trimmed to 1.5 mm)
lid_top = 21.0;

module tray() {
    o = PCB_OUTLINE;
    zb = -gap_b - floor_t;
    difference() {
        union() {
            // floor + walls round the board (the ECU edge stays open for the plugs' tails)
            translate([0, 0, zb]) linear_extrude(floor_t) offset(delta = wall) board2d();
            difference() {
                translate([0, 0, zb]) linear_extrude(PCB_T - zb) offset(delta = wall) board2d();
                translate([0, 0, zb - 1]) linear_extrude(PCB_T - zb + 2) offset(delta = 0.15) board2d();
                translate([o[0] - 5, o[1] - 5, zb - 1]) cube([o[2] - o[0] + 10, 5.01, 20]);
            }
            // plateau under the header: takes the plug insertion force (about 100 N)
            translate([-117.0, HDR_BACK + 1.0, zb]) cube([HDR_CUT + 116.5, HDR_MOUTH - HDR_BACK - 1.6, -zb]);
            // bosses under the M2 holes
            for (m = PCB_MOUNT) translate([m[0], m[1], zb]) cylinder(d = 5.0, h = -zb, $fn = 24);
            // ledge along the ECU edge
            translate([o[0] - 0.5, o[1], zb]) cube([o[2] - o[0] + 1.0, 1.2, -zb]);
        }
        for (m = PCB_MOUNT) translate([m[0], m[1], zb + 0.8]) cylinder(d = 1.7, h = 10, $fn = 16);
        // header pegs poke through the board
        for (h = PCB_HOLES) if (h[4] == 0 && h[2] > 3) translate([h[0], h[1], zb + floor_t]) cylinder(d = h[2] + 1, h = 10, $fn = 24);
        // ziptie slot (backup to the latch, as before)
        translate([-62, o[1] + 4, zb - 1]) cube([ziptie_w, 1.8, 10]);
    }
}

module lid() {
    o = PCB_OUTLINE;
    y0 = PLUG_REAR + 0.6;             // open ECU face: the tails come in here
    y1 = HDR_BACK;                    // butts against the header's back face
    difference() {
        union() {
            translate([LID_X0, y0, lid_top - 1.6]) cube([o[2] + wall - LID_X0, y1 - y0 + 2.0, 1.6]);
            // left end wall (with the screw) sits on the board; the right one on the tray wall
            translate([LID_X0, y0, PCB_T]) cube([7.0, y1 - y0, lid_top - PCB_T]);
            translate([o[2], y0, PCB_T]) cube([wall, y1 - y0, lid_top - PCB_T]);
            // a lip over the header's top edge keeps the lid from rocking
            translate([-113.4, y1, lid_top - 1.6]) cube([HDR_CUT + 113.4, 2.0, 1.6]);
        }
        for (m = PCB_MOUNT) if (m[1] > y0 && m[1] < y1) {
            translate([m[0], m[1], PCB_T - 1]) cylinder(d = 2.4, h = 40, $fn = 16);
            translate([m[0], m[1], lid_top - 2.5]) cylinder(d = 4.2, h = 10, $fn = 24);
        }
        // embossed legend
        for (t = [["E8", -94.2], ["E6", -62.4], ["E5", -27.1]])
            translate([t[1], -0.5, lid_top - 0.4]) linear_extrude(1)
                text(t[0], size = 3.2, halign = "center", valign = "center");
    }
}

// ---- scenes -------------------------------------------------------------
module ecu_face() {                  // context only: the ECU's header face
    color("#8a8f96", 0.35) translate([-121, -48, -4]) cube([124, 48 - 25.5 - 0.5, 30]);
}

if (part == "assembly") {
    pcb_board(); header_176122(); devkitc(); tails(); ecu_plugs(); harness_plugs();
    color("#2b2f36") tray();
    color("#2b2f36", 0.55) lid();
}
else if (part == "exploded") {
    pcb_board(); header_176122(); devkitc(); tails(); ecu_plugs();
    harness_plugs(35);
    color("#2b2f36") translate([0, 0, -14]) tray();
    color("#2b2f36", 0.8) translate([0, 0, 22]) lid();
}
else if (part == "board") { pcb_board(); header_176122(); devkitc(); tails(); }
else if (part == "tray") tray();
else if (part == "lid") translate([0, 0, lid_top]) rotate([180, 0, 0]) lid();
