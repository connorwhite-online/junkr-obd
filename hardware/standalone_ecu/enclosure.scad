// JUNKR standalone ECU: tray + lid for the 200 x 140 mm rev A board.
// Print in SLS/MJF PA12 (heat under the dash, fuel and oil nearby).
//
//   openscad -D part=\"tray\" -o tray.stl enclosure.scad
//   openscad -D part=\"lid\"  -o lid.stl  enclosure.scad
//   openscad -D part=\"assembly\" -o assembly.png enclosure.scad
//
// Board coordinates: origin top-left, x right, y down (as in KiCad). The
// harness header sits on the top (y = 0) edge and the plugs enter through a
// window in that wall; USB-C and microSD are on the left wall; the AUX
// Micro-Fit is vertical and comes out through the lid.
part = "assembly";
explode = 40;
$fn = 40;

BW = 200; BH = 140; BT = 1.6;      // board
GAP = 0.6;                         // board-to-wall clearance
WALL = 2.5; FLOOR = 2.5;
STANDOFF = 6;                      // board underside to floor (THT leads, relay pins)
ABOVE = 27;                        // floor of the lid to board top: tallest part + margin
HEADER_H = 24;                     // TE 176122-6 shroud height above the board [VERIFY]
LIP = 2;                           // lid tongue into the tray
HOLES = [[4, 46], [BW - 4, BH - 4], [4, BH - 4], [BW - 4, 30]];

IW = BW + 2 * GAP; IH = BH + 2 * GAP;
OW = IW + 2 * WALL; OH = IH + 2 * WALL;
TRAY_H = FLOOR + STANDOFF + BT + ABOVE * 0.55;   // split line roughly mid-height

// board (x, y) -> tray coordinates
function bx(x) = WALL + GAP + x;
function by(y) = WALL + GAP + (BH - y);     // flip so the header edge is at the back (+y)
BOARD_Z = FLOOR + STANDOFF;

module rounded_box(w, h, z, r = 3) {
    hull() for (x = [r, w - r], y = [r, h - r]) translate([x, y, 0]) cylinder(r = r, h = z);
}

module window(x0, x1, z0, z1, side) {
    // side: "back" (header edge, board y = 0) or "left" (board x = 0); x0/x1 are board coordinates
    if (side == "back")
        translate([bx(x0), OH - WALL - 1, z0]) cube([x1 - x0, WALL + 2, z1 - z0]);
    if (side == "left")
        translate([-1, by(x1), z0]) cube([WALL + 2, x1 - x0, z1 - z0]);
}

module cutouts() {
    // Harness header mouth: x 41.1 .. 159.2 on the board, shroud above the board.
    window(40.0, 160.5, BOARD_Z + BT - 0.5, BOARD_Z + BT + HEADER_H + 0.8, "back");
    // USB-C (y 56.7 .. 67.3) and microSD (y 80 .. 96) on the left wall.
    window(57.5, 66.5, BOARD_Z + BT - 0.3, BOARD_Z + BT + 3.8, "left");
    window(82.0, 94.0, BOARD_Z + BT - 0.3, BOARD_Z + BT + 2.2, "left");
}

module tray() {
    difference() {
        union() {
            difference() {
                rounded_box(OW, OH, TRAY_H);
                translate([WALL, WALL, FLOOR]) cube([IW, IH, TRAY_H]);
            }
            // standoffs with M3 heat-set insert bores
            for (h = HOLES) translate([bx(h[0]), by(h[1]), 0]) cylinder(d = 8, h = BOARD_Z);
            // board ledge ribs along the long walls
            for (x = [30, 100, 170]) {
                translate([bx(x) - 3, WALL, 0]) cube([6, 2, BOARD_Z]);
                translate([bx(x) - 3, OH - WALL - 2, 0]) cube([6, 2, BOARD_Z]);
            }
            // mounting ears for the dash bracket
            for (y = [20, OH - 20]) for (side = [0, 1])
                translate([side ? OW - 1 : -12, y - 8, 0]) difference() {
                    cube([13, 16, 4]);   // 1 mm into the wall so it fuses with the tray
                    translate([side ? 7 : 6, 8, -1]) cylinder(d = 5.5, h = 6);
                }
        }
        for (h = HOLES) translate([bx(h[0]), by(h[1]), FLOOR]) cylinder(d = 4.2, h = BOARD_Z);   // insert bore
        cutouts();
        // label recess on the floor
        translate([OW / 2, OH / 2, -0.01]) mirror([1, 0, 0])
            linear_extrude(0.6) text("JUNKR ECU rev A", size = 8, halign = "center", valign = "center");
    }
}

TOP = FLOOR + STANDOFF + BT + ABOVE;      // underside of the lid's top plate

// Built where it sits on the tray; "lid" export flips it onto its top face.
module lid() {
    difference() {
        union() {
            translate([0, 0, TOP]) rounded_box(OW, OH, FLOOR);
            // walls that complete the box above the split
            translate([0, 0, TRAY_H]) difference() {
                rounded_box(OW, OH, TOP - TRAY_H + 0.01);
                translate([WALL, WALL, -1]) cube([IW, IH, TOP]);
            }
            // tongue that drops inside the tray wall
            translate([WALL + 0.25, WALL + 0.25, TRAY_H - LIP]) difference() {
                cube([IW - 0.5, IH - 0.5, LIP + 1]);
                translate([1.6, 1.6, -1]) cube([IW - 3.7, IH - 3.7, LIP + 3]);
            }
            // bosses down to the board for the M3 screws (clamp the board)
            for (h = HOLES) translate([bx(h[0]), by(h[1]), BOARD_Z + BT]) cylinder(d = 8, h = TOP - BOARD_Z - BT + 0.01);
            // stiffening ribs under the top plate
            for (x = [OW / 3, 2 * OW / 3]) translate([x - 1, WALL, TOP - 6]) cube([2, IH, 6.01]);
        }
        for (h = HOLES) translate([bx(h[0]), by(h[1]), 0]) {
            cylinder(d = 3.4, h = TOP + FLOOR + 1);
            translate([0, 0, TOP + FLOOR - 3.2]) cylinder(d = 6.2, h = 4);   // counterbore
        }
        // AUX Micro-Fit (board x 166.9 .. 190.1, y 3 .. 12.8), through the lid
        translate([bx(166.4), by(13.3), TOP - 10]) cube([24.2, 10.8, FLOOR + 11]);
        // LED windows: RUN / FAULT / ARM / supervisor
        for (p = [[118, 112], [122, 112], [126, 112], [138, 100]])
            translate([bx(p[0]), by(p[1]), TOP - 1]) cylinder(d = 2.2, h = FLOOR + 2);
        cutouts();
        // the ribs must clear the header shroud
        translate([bx(40), by(26), BOARD_Z]) cube([121, 27, HEADER_H + BT + 1]);
        translate([OW / 2, OH / 2, TOP + FLOOR - 0.6])
            linear_extrude(1) text("JUNKR", size = 14, halign = "center", valign = "center");
    }
}

module board_ghost() {
    color("darkgreen") translate([bx(0), by(BH), BOARD_Z]) cube([BW, BH, BT]);
    color("silver") translate([bx(41.1), by(25.9), BOARD_Z + BT]) cube([118.1, 26.5, HEADER_H]);
}

if (part == "tray") tray();
if (part == "lid") translate([0, OH, TOP + FLOOR]) rotate([180, 0, 0]) lid();
if (part == "assembly") {
    color("#2b2b2b") tray();
    board_ghost();
    color("#3b3b3b", 0.55) translate([0, 0, explode]) lid();
}
