// =====================================================================
//  PCB interposer (rev A): one board between the ECU and all three
//  harness plugs, plus its two printed parts (tray + lid).
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
    color("#3a3a3a") {
        for (i = [0 : 2]) {
            s = SECS[i];
            sec_T(i, HDR_MOUTH - sh_len(s))
                intersection() {
                    male_shroud(s, false);
                    translate([-100, -50, sh_len(s) - (HDR_MOUTH - HDR_BACK)]) cube([200, 100, 100]);
                }
        }
        // board-lock ears at both ends and the centre
        for (e = [[-117.8, -113.4], [-4.4, 0.0]])
            translate([e[0], HDR_BACK, PCB_T]) cube([e[1] - e[0], 8.5, 5.5]);
    }
    // right-angle pins: out of the back face, bend down into the board
    color("#c8a24a")
        for (i = [0 : 2]) {
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
    for (i = [0 : 2]) color("#d9d4c7") sec_T(i, PLUG_REAR - plug_total(SECS[i])) oem_plug(i);
}

module harness_plugs(gap = 0) {
    for (i = [0 : 2]) color("#5d6066") sec_T(i, HDR_MOUTH - insert_len(SECS[i]) + gap) oem_plug(i);
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

module pcb_board() {
    o = PCB_OUTLINE;
    color("#1f5d3a") difference() {
        translate([o[0], o[1], 0]) cube([o[2] - o[0], o[3] - o[1], PCB_T]);
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

module j5_header() {                 // 2x7 right-angle, pins out of the -X edge
    color("#151515") translate([-124.3, -20.6, PCB_T]) cube([2.5, 16.0, 5.1]);
    color("#c8a24a") for (k = [0 : 13])
        let (col = k % 2, row = floor(k / 2), x = -119.8 - col * 2.54, y = -(4.0 + row * 2.54))
            translate([-130.5, y - 0.32, PCB_T + 1.27 + col * 2.54]) cube([x + 130.5, 0.64, 0.64]);
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
            // floor + three walls (the ECU edge stays open for the plugs' tails)
            translate([o[0] - wall, o[1], zb]) cube([o[2] - o[0] + 2 * wall, o[3] - o[1] + wall, floor_t]);
            for (x = [o[0] - wall, o[2] - 0.01]) translate([x, o[1], zb]) cube([wall + 0.01, o[3] - o[1] + wall, PCB_T - zb]);
            translate([o[0] - wall, o[3] - 0.01, zb]) cube([o[2] - o[0] + 2 * wall, wall + 0.01, PCB_T - zb]);
            // plateau under the header: takes the plug insertion force (about 100 N)
            translate([-117.0, HDR_BACK + 1.0, zb]) cube([116.0, HDR_MOUTH - HDR_BACK - 1.6, -zb]);
            // bosses under the M2 holes
            for (m = PCB_MOUNT) translate([m[0], m[1], zb]) cylinder(d = 5.0, h = -zb, $fn = 24);
            // ledge along the ECU edge
            translate([o[0] - 0.5, o[1], zb]) cube([o[2] - o[0] + 1.0, 1.2, -zb]);
        }
        for (m = PCB_MOUNT) translate([m[0], m[1], zb + 0.8]) cylinder(d = 1.7, h = 10, $fn = 16);
        // header pegs poke through the board
        for (h = PCB_HOLES) if (h[4] == 0 && h[2] > 3) translate([h[0], h[1], zb + floor_t]) cylinder(d = h[2] + 1, h = 10, $fn = 24);
        // J5 pins leave over the left wall
        translate([o[0] - wall - 1, -21, PCB_T - 0.01]) cube([wall + 2, 18, 10]);
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
            translate([o[0] - wall, y0, lid_top - 1.6]) cube([o[2] - o[0] + 2 * wall, y1 - y0 + 2.0, 1.6]);
            // end walls with the screw ears
            for (x = [o[0] - wall, o[2] - 4.0]) translate([x, y0, PCB_T]) cube([4.0 + wall, y1 - y0, lid_top - PCB_T]);
            // a lip over the header's top edge keeps the lid from rocking
            translate([-113.4, y1, lid_top - 1.6]) cube([113.0, 2.0, 1.6]);
        }
        for (m = PCB_MOUNT) if (m[1] > y0 && m[1] < y1) {
            translate([m[0], m[1], PCB_T - 1]) cylinder(d = 2.4, h = 40, $fn = 16);
            translate([m[0], m[1], lid_top - 2.5]) cylinder(d = 4.2, h = 10, $fn = 24);
        }
        // J5 body sits under the left end wall
        translate([o[0] - wall - 1, -21, PCB_T - 0.01]) cube([7.5, 17, 6.0]);
        // embossed legend
        translate([-60, -0.5, lid_top - 0.4]) linear_extrude(1)
            text("E8            E6                    E5", size = 3.2, halign = "center", valign = "center");
    }
}

// ---- scenes -------------------------------------------------------------
module ecu_face() {                  // context only: the ECU's header face
    color("#8a8f96", 0.35) translate([-121, -48, -4]) cube([124, 48 - 25.5 - 0.5, 30]);
}

if (part == "assembly") {
    pcb_board(); header_176122(); j5_header(); tails(); ecu_plugs(); harness_plugs();
    color("#2b2f36") tray();
    color("#2b2f36", 0.55) lid();
}
else if (part == "exploded") {
    pcb_board(); header_176122(); j5_header(); tails(); ecu_plugs();
    harness_plugs(35);
    color("#2b2f36") translate([0, 0, -14]) tray();
    color("#2b2f36", 0.8) translate([0, 0, 22]) lid();
}
else if (part == "board") { pcb_board(); header_176122(); j5_header(); tails(); }
else if (part == "tray") tray();
else if (part == "lid") translate([0, 0, lid_top]) rotate([180, 0, 0]) lid();
