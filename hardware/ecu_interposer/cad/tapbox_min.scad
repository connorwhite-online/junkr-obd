// =====================================================================
//  Minimal tap box (stage 1, drawing KZI-121): holds one
//  ESP32-S3-DevKitC-1 with its pins pointing DOWN onto the 13 tap leads.
//  No PCB, no screws. The board rests on two saddles, two lips locate it
//  sideways, and two fingers on the snap lid hold it down (stick a 1 mm
//  foam pad on each finger if the board rattles).
//
//  openscad -o ../stl/tapbox_min_base.stl -D 'part="base"' tapbox_min.scad
//  openscad -o ../stl/tapbox_min_lid.stl  -D 'part="lid"'  tapbox_min.scad
// =====================================================================

part = "base";            // "base" | "lid" | "assembly"
$fn = 40;

// --- board (DevKitC-1 outline: check the one you buy) [VERIFY] ---
board_l   = 62.7;          // only sets where the fingers land; length is not captured
board_w   = 25.4;
board_t   = 1.6;
hdr_in    = 2.9;           // header + Dupont housing band, measured in from each long edge
under_h   = 16.0;          // room under the board for Dupont housings + wire bend

// --- box ---
L = 96; W = 48; H = 30;    // outer, base only (lid adds 2)
wall = 2.0; floor_t = 2.0;
clr = 0.3;                 // lid fit clearance per side
lid_t = 2.0;

iL = L - 2 * wall; iW = W - 2 * wall;
sup_z = floor_t + under_h;                 // board underside height
bx0   = wall + 2.0;                        // board USB end, 2 mm from the inner wall
cy    = W / 2;

module saddle(x) {
    // centre pad between the two header bands
    pad = board_w - 2 * hdr_in - 1.0;
    translate([x - 1.5, cy - pad / 2, 0]) cube([3, pad, sup_z]);
    // side lips just outside the board edge, 2 mm above the support
    for (s = [-1, 1])
        translate([x - 1.5, cy + s * (board_w / 2 + clr) + (s < 0 ? -1.6 : 0), 0])
            cube([3, 1.6, sup_z + 2.0]);
}

module base() {
    difference() {
        union() {
            difference() {
                cube([L, W, H]);
                translate([wall, wall, floor_t]) cube([iL, iW, H]);
            }
            for (x = [bx0 + 10, bx0 + board_l - 12]) saddle(x);
            // mounting ears for zip ties or M4 screws
            for (x = [12, L - 12]) for (s = [0, 1])
                translate([x - 7, s ? W : -10, 0]) difference() {
                    cube([14, 10, 3]);
                    translate([7, s ? 5.5 : 4.5, -1]) cylinder(d = 4.5, h = 5);
                }
        }
        // USB-C end: both ports of the DevKitC, at board-top height
        translate([-1, cy - 13, sup_z + board_t - 1.0]) cube([wall + 2, 26, 8]);
        // tap harness end (antenna end): 12 x 8 slot near the floor
        translate([L - wall - 1, cy - 6, floor_t + 2]) cube([wall + 2, 12, 8]);
        // snap bumps' sockets for the lid (2 per long side)
        for (x = [L * 0.25, L * 0.75]) for (y = [wall - 0.6, W - wall + 0.6])
            translate([x, y, H - 2.5]) rotate([0, 90, 0]) cylinder(d = 1.6, h = 8, center = true);
    }
    // cable-tie bar outside the harness slot
    translate([L, cy - 9, floor_t]) difference() {
        cube([6, 18, 3]);
        translate([1.5, 3, -1]) cube([3, 12, 5]);
    }
}

module lid() {
    // printed top face down
    cube([L, W, lid_t]);
    lip = 1.2;
    translate([wall + clr, wall + clr, lid_t]) difference() {
        cube([iL - 2 * clr, iW - 2 * clr, 4]);
        translate([lip, lip, -1]) cube([iL - 2 * clr - 2 * lip, iW - 2 * clr - 2 * lip, 6]);
        // clear the USB slot and harness slot
        translate([-1, cy - wall - clr - 13, -1]) cube([lip + 2, 26, 6]);
    }
    // snap bumps
    for (x = [L * 0.25, L * 0.75]) for (y = [wall + clr, W - wall - clr])
        translate([x, y, lid_t + 2.5]) rotate([0, 90, 0]) cylinder(d = 1.2, h = 7, center = true);
    // hold-down fingers: stop 0.5 mm above the board top (about 4 mm of parts on top)
    finger_len = (H - (sup_z + board_t + 4.0)) - 0.5;
    for (x = [bx0 + 10, bx0 + board_l - 12])
        translate([x - 3, cy - 3, lid_t]) cube([6, 6, finger_len]);
}

if (part == "base") base();
else if (part == "lid") lid();
else {
    base();
    color("Green", 0.6) translate([bx0, cy - board_w / 2, sup_z]) cube([board_l, board_w, board_t]);
    color("Gray", 0.4) translate([0, W, H + lid_t]) rotate([180, 0, 0]) lid();
}
