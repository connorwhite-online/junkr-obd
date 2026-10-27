// =====================================================================
//  Climate panel replacement: every printable part
//
//  openscad -D 'part="faceplate"' panel.scad
//  parts: faceplate, switch_plate, cap (cap_i picks the button), rear_cover,
//         sleeve, fit_frame, cap_test, assembly (preview only)
// =====================================================================

include <params.scad>

part  = "assembly";
cap_i = 0;

// ---- Helpers ---------------------------------------------------------------
module rrect(w, h, r) {
    offset(r = r) square([w - 2 * r, h - 2 * r], center = true);
}

module rbox(w, h, r, t) { linear_extrude(t) rrect(w, h, r); }

skirt_ow = open_w - 2 * fit_clr;
skirt_oh = open_h - 2 * fit_clr;
skirt_or = max(open_r - fit_clr, 0.5);
skirt_iw = skirt_ow - 2 * skirt_wall;
skirt_ih = skirt_oh - 2 * skirt_wall;
skirt_ir = max(skirt_or - skirt_wall, 0.5);
back_z   = -face_t - skirt_d;

dial_hole = (sleeve ? sleeve_od : dial_d) + 2 * dial_clr;
dial_front = dial_proud;
dial_back  = dial_proud - dial_h;
cup_top    = dial_back + dial_grip_from;    // cup grips the base up to here

// ---- Faceplate -------------------------------------------------------------
module face_outline() { rrect(face_w, face_h, face_r); }

module faceplate() {
    difference() {
        union() {
            // face with a chamfered front edge
            hull() {
                translate([0, 0, -face_t]) rbox(face_w, face_h, face_r, face_t - face_chamfer);
                translate([0, 0, -face_chamfer])
                    rbox(face_w - 2 * face_chamfer, face_h - 2 * face_chamfer,
                         max(face_r - face_chamfer, 0.5), face_chamfer);
            }
            // skirt into the dash opening
            translate([0, 0, back_z]) linear_extrude(skirt_d)
                difference() { rrect(skirt_ow, skirt_oh, skirt_or); rrect(skirt_iw, skirt_ih, skirt_ir); }
            // plate standoffs
            for (p = posts) translate([p[0], p[1], plate_z]) cylinder(d = post_d, h = -face_t - plate_z + 0.01);
            // rear-cover bosses in the four inside corners
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx * (skirt_iw / 2 - 2.5), sy * (skirt_ih / 2 - 2.5), back_z])
                    cylinder(d = 6, h = plate_z - mx_plate - 1 - back_z);
            // mounting ears
            for (e = ears) ear(e);
        }
        // key cap openings
        for (b = buttons) translate([b[1], b[2], -face_t - 1])
            rbox(cap_s + 2 * cap_clr, cap_s + 2 * cap_clr, cap_r + cap_clr, face_t + 2);
        // knob opening
        translate([dial_x, dial_y, -face_t - 1]) cylinder(d = dial_hole, h = face_t + 2);
        // pilots
        for (p = posts) translate([p[0], p[1], plate_z - 1]) cylinder(d = post_pilot, h = 10);
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx * (skirt_iw / 2 - 2.5), sy * (skirt_ih / 2 - 2.5), back_z - 1])
                cylinder(d = post_pilot, h = 10);
    }
}

// An ear: a flat tab off the nearest skirt side, hole along Z.
module ear(e) {
    side = sign(e[0]);
    x0   = side * (skirt_ow / 2 - 0.5);
    translate([0, 0, -ear_z - ear_t]) difference() {
        hull() {
            translate([x0, e[1] - 5, 0]) cube([0.01, 10, ear_t]);
            translate([e[0], e[1], 0]) cylinder(d = 10, h = ear_t);
        }
        translate([e[0], e[1], -1]) cylinder(d = ear_hole, h = ear_t + 2);
    }
    // gusset back to the skirt
    translate([0, 0, -ear_z - ear_t]) hull() {
        translate([x0, e[1] - 1, 0]) cube([0.01, 2, ear_t + 6]);
        translate([x0 + side * 5, e[1] - 1, 0]) cube([0.01, 2, ear_t]);
    }
}

// ---- Switch plate + Dial cup -----------------------------------------------
module switch_plate() {
    pw = skirt_iw - 2 * 0.3;
    ph = skirt_ih - 2 * 0.3;
    cup_od = dial_d + 2 * 0.15 + 2 * cup_wall;
    difference() {
        union() {
            translate([0, 0, plate_z - mx_plate]) rbox(pw, ph, max(skirt_ir - 0.3, 0.5), mx_plate);
            // cup round the Dial's fixed base, hanging off the plate. Where the
            // skirt is too low for a full ring, the skirt wall is the cup.
            intersection() {
                translate([dial_x, dial_y, dial_back - cup_floor])
                    cylinder(d = cup_od, h = cup_top - (dial_back - cup_floor));
                translate([0, 0, -100]) rbox(pw, ph, max(skirt_ir - 0.3, 0.5), 200);
            }
        }
        // switch holes
        for (b = buttons) translate([b[1], b[2], plate_z - mx_plate - 1])
            linear_extrude(mx_plate + 2) square(mx_hole, center = true);
        // cup bore: Dial body, plus a clear path up through the plate
        translate([dial_x, dial_y, dial_back]) cylinder(d = dial_d + 0.3, h = 50);
        // cable slot in the cup floor
        translate([dial_x, dial_y - dial_d / 4, dial_back - cup_floor - 1])
            linear_extrude(cup_floor + 2) square(cable_slot, center = true);
        // big centre hole in the floor too (Dial back stays cool, wires route)
        translate([dial_x, dial_y + 6, dial_back - cup_floor - 1]) cylinder(d = 22, h = cup_floor + 2);
        // optional screws up into the Dial's back shell
        for (h = dial_back_screws) translate([dial_x + h[0], dial_y + h[1], dial_back - cup_floor - 1])
            cylinder(d = 2.3, h = cup_floor + 2);
        // standoff screws
        for (p = posts) translate([p[0], p[1], plate_z - mx_plate - 1]) cylinder(d = post_screw, h = mx_plate + 2);
        // notches for the rear-cover bosses
        for (sx = [-1, 1], sy = [-1, 1])
            translate([sx * (skirt_iw / 2 - 2.5), sy * (skirt_ih / 2 - 2.5), plate_z - 5])
                cylinder(d = 7, h = 10);
    }
    // crush ribs: the Dial base presses in and stays put
    for (a = [0 : 60 : 359]) translate([dial_x, dial_y, dial_back]) rotate([0, 0, a])
        translate([(dial_d + 0.3) / 2 - rib_h, -0.6, 0]) cube([rib_h + 0.2, 1.2, cup_top - dial_back]);
}

// ---- Key cap (Cherry MX cross stem) ------------------------------------------
module cap(label) {
    cross_l = 4.1;  cross_w = 1.25;   // MX stem, plus a little for print swell
    difference() {
        union() {
            hull() {
                rbox(cap_s, cap_s, cap_r, cap_h - 0.8);
                translate([0, 0, cap_h - 0.8]) rbox(cap_s - 1.2, cap_s - 1.2, max(cap_r - 0.6, 0.3), 0.8);
            }
            // stem socket boss below the cap
            translate([0, 0, -3.8]) cylinder(d = 5.5, h = 3.81);
        }
        // cross socket
        translate([0, 0, -3.9]) {
            linear_extrude(3.8) square([cross_l, cross_w], center = true);
            linear_extrude(3.8) square([cross_w, cross_l], center = true);
        }
        // hollow the body from below, keeping a 1.6 mm top and 1.2 mm walls
        translate([0, 0, -0.01]) difference() {
            rbox(cap_s - 2.4, cap_s - 2.4, max(cap_r - 1.2, 0.3), cap_h - 1.6);
            cylinder(d = 5.5, h = cap_h);
        }
        // legend
        if (label != "")
            translate([0, 0, cap_h - legend_d]) linear_extrude(legend_d + 0.1)
                text(label, size = len(label) > 4 ? legend_size * 0.8 : legend_size,
                     font = "Liberation Sans:style=Bold", halign = "center", valign = "center");
    }
}

// ---- Rear cover ------------------------------------------------------------
module rear_cover() {
    difference() {
        translate([0, 0, back_z - cover_t]) rbox(skirt_ow, skirt_oh, skirt_or, cover_t);
        translate([0, 0, back_z - cover_t - 1]) linear_extrude(cover_t + 2) {
            translate([-skirt_iw / 4 - 10, 0]) rrect(grommet[0], grommet[1], 2);
            for (sx = [-1, 1], sy = [-1, 1])
                translate([sx * (skirt_iw / 2 - 2.5), sy * (skirt_ih / 2 - 2.5)]) circle(d = post_screw);
            for (h = perf_holes) translate([skirt_iw / 4 + 8 + h[0], h[1]]) circle(d = 2.6);
            // USB-C lead to the Dial
            translate([dial_x, dial_y - dial_d / 4]) rrect(14, 8, 2);
        }
    }
}

// ---- Optional sleeve over the Dial ring -----------------------------------
module dial_sleeve() {
    difference() {
        cylinder(d = sleeve_od, h = sleeve_h, $fn = 120);
        translate([0, 0, -1]) cylinder(d = dial_d + 0.2, h = sleeve_h + 2);
        for (a = [0 : 10 : 359]) rotate([0, 0, a]) translate([sleeve_od / 2 + 0.4, 0, -1])
            cylinder(d = 2, h = sleeve_h + 2, $fn = 12);
    }
}

// ---- Fit frame: face outline + skirt, nothing else. Print it first. --------
module fit_frame() {
    difference() {
        union() {
            translate([0, 0, -1.2]) rbox(face_w, face_h, face_r, 1.2);
            translate([0, 0, -1.2 - 8]) linear_extrude(8)
                difference() { rrect(skirt_ow, skirt_oh, skirt_or); rrect(skirt_ow - 3, skirt_oh - 3, max(skirt_or - 1.5, 0.5)); }
        }
        translate([0, 0, -5]) rbox(face_w - 12, face_h - 12, 2, 10);
    }
}

// ---- Cap test: one MX hole and a cap, to dial in mx_clr and the stem fit ---
module cap_test() {
    difference() {
        translate([0, 0, 0]) rbox(26, 26, 2, mx_plate);
        translate([0, 0, -1]) linear_extrude(mx_plate + 2) square(mx_hole, center = true);
    }
    translate([30, 0, 3.8]) cap("A/C");
}

// ---- Assembly preview -------------------------------------------------------
module dial_dummy() {
    color("#2a2a2a") translate([dial_x, dial_y, dial_back]) cylinder(d = dial_d, h = dial_h - 2);
    color("#111") translate([dial_x, dial_y, dial_front - 2]) cylinder(d = dial_d - 6, h = 2);
    color("#e8eef0") translate([dial_x, dial_y, dial_front - 0.2]) cylinder(d = 32, h = 0.4);
}

module mx_dummy() {
    color("#555") translate([0, 0, plate_z - 5]) linear_extrude(5) square(14, center = true);
    color("#666") translate([0, 0, plate_z]) linear_extrude(mx_housing) square(14.6, center = true);
}

module assembly() {
    color("#3b3b3b") faceplate();
    color("#8a8a8a") switch_plate();
    color("#555", 0.6) rear_cover();
    for (b = buttons) translate([b[1], b[2], 0]) {
        mx_dummy();
        color("#d9d9d4") translate([0, 0, cap_proud - cap_h]) cap(b[0]);
    }
    dial_dummy();
}

module exploded(gap = 45) {
    color("#3b3b3b") faceplate();
    for (b = buttons) translate([b[1], b[2], gap]) color("#d9d9d4") translate([0, 0, cap_proud - cap_h]) cap(b[0]);
    translate([0, 0, gap]) dial_dummy();
    translate([0, 0, -gap]) {
        color("#8a8a8a") switch_plate();
        for (b = buttons) translate([b[1], b[2], 0]) mx_dummy();
    }
    translate([0, 0, -2.2 * gap]) color("#666") rear_cover();
}

// ---- Part selector ------------------------------------------------------------
// Printable parts are laid out flat on the bed, face down where it matters.
if (part == "faceplate")        rotate([180, 0, 0]) faceplate();
else if (part == "switch_plate") translate([0, 0, -(plate_z - mx_plate)]) switch_plate();
else if (part == "cap")          translate([0, 0, cap_h]) rotate([180, 0, 0]) cap(buttons[cap_i][0]);  // legend on the bed
else if (part == "rear_cover")   translate([0, 0, -(back_z - cover_t)]) rear_cover();
else if (part == "sleeve")       dial_sleeve();
else if (part == "fit_frame")    rotate([180, 0, 0]) fit_frame();
else if (part == "cap_test")     cap_test();
else if (part == "exploded")     exploded();
else if (part == "count")        echo(n_buttons = len(buttons));
else                             assembly();
