// =====================================================================
//  ECU interposer — parametric library
//
//  One "section" (E5 26-pin / E6 16-pin / E8 22-pin) = four printed parts:
//
//    ECU header  <==  female_plug  [chamber_half x2]  male_shroud  <==  OEM harness plug
//    (176122-6)       (receptacles,                   (male tabs; a copy of
//                      keyed like the                  the 176122-6 cavity)
//                      OEM plug)
//
//  Every pin passes straight through on a short pigtail inside the chamber.
//  Taps are spliced on in the chamber and leave through the side slot on a
//  ribbon to the tap box (ESP32 board). Nothing on the vehicle is cut.
//
//  Frame: X along the pin rows (centred on the section), Y up from the
//  header cavity floor, Z = mating axis.
// =====================================================================

include <params.scad>
include <sections.scad>

// ---------------------------------------------------------------------
// Derived geometry
// ---------------------------------------------------------------------
function cx(s, x) = x - s_len(s) / 2;                     // cavity-relative -> centred X
function row_y(t, r) = t == 2 ? ROW_Y_070[r] : ROW_Y_040[r];
function tab_proud(t) = t == 2 ? TAB_PROUD_070 : TAB_PROUD_040;

// female plug
function insert_len(s) = HDR_DEPTH - 0.2;                 // front face stops 0.2 short of the floor
function plug_len(s)   = insert_len(s) + plug_tail;

// male shroud = header copy
function sh_w(s)  = s_len(s) + 2 * HDR_WALL;
function sh_y0(s) = -HDR_WALL - HDR_GROOVE_D;                // floor wall stays HDR_WALL under the grooves
function sh_y1(s) = HDR_SLOT_TOP + HDR_WALL;
function sh_len(s) = shroud_floor + HDR_DEPTH;

// common flange that the chamber clamps (same on both ends)
flange_t     = 2.0;
flange_extra = 1.6;
function fl_w(s)  = sh_w(s) + 2 * flange_extra;
function fl_y0(s) = sh_y0(s) - flange_extra;
function fl_y1(s) = sh_y1(s) + flange_extra;

// ---------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------
module box_xy(x0, x1, y0, y1, z0, z1) {
    translate([x0, y0, z0]) cube([x1 - x0, y1 - y0, z1 - z0]);
}

module for_pins(s) {
    for (col = s_cols(s), r = [0, 1])
        translate([cx(s, col[0]), row_y(col[1], r), 0])
            let ($t = term(col[1]), $code = col[1], $out = (r == 0) ? 1 : -1)
                children();
}

// Thinnest wall left between neighbouring receptacle cavities.
function col_gap(s, i) = s_cols(s)[i + 1][0] - s_cols(s)[i][0]
    - (body_w(term(s_cols(s)[i][1])) + body_w(term(s_cols(s)[i + 1][1]))) / 2 - 2 * clr;
function web_x(s) = min([ for (i = [0 : len(s_cols(s)) - 2]) col_gap(s, i) ]);
module check_walls(s) {
    wx = web_x(s);
    echo(str(s_name(s), ": thinnest cavity web = ", wx, " mm (min_wall = ", min_wall, ")"));
    if (wx < min_wall)
        echo(str("WARNING: ", s_name(s), " webs below min_wall; print terminal parts in resin/MJF or tune T040/T070/clr"));
}

// ---------------------------------------------------------------------
// Terminal cavities
// ---------------------------------------------------------------------

// Receptacle (female) cavity. Front face at z = 0, terminal inserted from +Z.
// Tab width runs along X [MEASURE: confirm tab orientation on the header].
module receptacle_cavity(t, out, length) {
    bw = body_w(t) + 2 * clr;
    bh = body_h(t) + 2 * clr;
    tw = tab_w(t) + 2 * clr + 0.2;
    tt = tab_t(t) + 2 * clr + 0.3;
    translate([-tw / 2, -tt / 2, -1]) cube([tw, tt, face_wall + 2]);
    hull() {                                    // 45° lead-in for the tab
        translate([-tw / 2, -tt / 2, 0.4]) cube([tw, tt, 0.01]);
        translate([-tw / 2 - 0.4, -tt / 2 - 0.4, -0.01]) cube([tw + 0.8, tt + 0.8, 0.01]);
    }
    translate([-bw / 2, -bh / 2, face_wall]) cube([bw, bh, length]);
    // lance relief: the terminal's spring lance pops out here; the step at
    // z = face_wall + lance_from_front is what stops it backing out
    lh = lance_h(t);
    translate([-bw / 2 + 0.2, out > 0 ? bh / 2 - 0.01 : -bh / 2 - lh, face_wall])
        cube([bw - 0.4, lh + 0.01, lance_ff(t)]);
}

// Male tab cavity in the shroud's back wall. Floor top at z = top; the
// terminal goes in from below and its tab stands up into the cavity.
module tab_cavity(t, out, top, bottom) {
    bw = body_w(t) + 2 * clr;
    bh = body_h(t) + 2 * clr;
    tw = tab_w(t) + 2 * clr;
    tt = tab_t(t) + 2 * clr;
    web = 1.0;                                  // floor web the tab shoulder seats against
    translate([-tw / 2, -tt / 2, top - web - 0.01]) cube([tw, tt, web + 1]);
    translate([-bw / 2, -bh / 2, bottom - 1]) cube([bw, bh, top - web - bottom + 1]);
    lh = lance_h(t);
    translate([-bw / 2 + 0.2, out > 0 ? bh / 2 - 0.01 : -bh / 2 - lh, top - web - lance_ff(t)])
        cube([bw - 0.4, lh + 0.01, lance_ff(t)]);
}

// ---------------------------------------------------------------------
// Female plug: goes into the ECU's 176122-6 header. Print front face down.
// ---------------------------------------------------------------------
module plug_envelope(s, z0, z1) {
    L = s_len(s);
    difference() {
        union() {
            box_xy(-L / 2 + slide_clr, L / 2 - slide_clr, slide_clr, HDR_CAV_H - slide_clr, z0, z1);
            // key ridges that ride in the header's top-wall slots
            // (the two centre slots keep only their outer part: the lock tines use the rest)
            for (sl = s_slots(s))
                let (x0 = cx(s, sl[0]) + key_clr, x1 = cx(s, sl[1]) - key_clr,
                     a = plug_latch && x1 > -latch_guard_x && x1 <= 0 ? -latch_guard_x : x1,
                     b = plug_latch && x0 < latch_guard_x && x0 >= 0 ? latch_guard_x : x0)
                    if (b < a)
                        box_xy(b, a, HDR_CAV_H - slide_clr - 0.01, HDR_SLOT_TOP - key_clr, z0, z1);
            // bottom keys that ride in the header's floor grooves
            for (g = s_grooves(s))
                box_xy(cx(s, g[0]) + key_clr, cx(s, g[1]) - key_clr, -HDR_GROOVE_D + key_clr, slide_clr + 0.01, z0, z1);
        }
        // grooves for the polarising rails on both end walls
        rl = s_rail(s);
        for (sx = [-1, 1])
            translate([sx > 0 ? L / 2 - rl[2] - key_clr - slide_clr : -L / 2 - 1, rl[0] - key_clr, z0 - 1])
                cube([rl[2] + key_clr + slide_clr + 1, rl[1] - rl[0] + 2 * key_clr, z1 - z0 + 2]);
    }
}

module female_plug(s, with_flange = true) {
    Lp = plug_len(s);
    check_walls(s);
    difference() {
        union() {
            plug_envelope(s, 0, Lp);
            if (with_flange) box_xy(-fl_w(s) / 2, fl_w(s) / 2, fl_y0(s), fl_y1(s), Lp, Lp + flange_t);
        }
        for_pins(s) receptacle_cavity($t, $out, Lp + flange_t + 2);
        if (plug_latch && len(s_slots(s)) > 0)       // relief under the tines (not on the fit coupon)
            box_xy(-latch_x[1] - 0.2, latch_x[1] + 0.2, HDR_CAV_H - slide_clr - latch_pocket,
                   HDR_CAV_H, latch_root, latch_end + 0.5);
        // pin-1 marker: on the +X, upper-row corner, which is Toyota pin 1
        // (see pin_no in sections.scad); top-right seen from the wire side
        if (with_flange)
            translate([fl_w(s) / 2 - 1.5, fl_y1(s) - 1.5, Lp + flange_t - 0.4]) cylinder(d = 1.2, h = 1);
    }
    if (plug_latch && len(s_slots(s)) > 0) plug_latch_arm();
}

// Lock tines copied from the OEM 175110-6: rooted at the front, free at the
// back, joined by a crossbar with a press tab outside the header mouth.
// Press the tab to drop the hooks and pull the plug.
module plug_latch_arm() {
    yb = latch_top - latch_t;
    zl = latch_lock_z;
    for (sx = [-1, 1]) mirror([sx < 0 ? 1 : 0, 0, 0]) {
        box_xy(latch_x[0], latch_x[1], HDR_CAV_H - slide_clr - latch_pocket - 0.01, latch_top, 0, latch_root);
        box_xy(latch_x[0], latch_x[1], yb, latch_top, latch_root - 0.01, latch_end);
        // hook: 45° ramp facing the front, square lock face facing the back
        translate([latch_x[0], 0, 0]) multmatrix([[0, 0, 1, 0], [0, 1, 0, 0], [1, 0, 0, 0]])   // 2D (z, y) -> 3D, extruded along x
            linear_extrude(latch_x[1] - latch_x[0])
                polygon([[zl - latch_flat - latch_ramp, latch_top - 0.01], [zl - latch_flat, latch_hook_top],
                         [zl, latch_hook_top], [zl, latch_top - 0.01]]);
    }
    box_xy(-latch_x[1], latch_x[1], yb, latch_top, latch_end - latch_tab_l, latch_end);
    box_xy(-latch_x[1] + 0.4, latch_x[1] - 0.4, latch_top - 0.01, latch_hook_top - 0.2, latch_end - latch_tab_l, latch_end);
}

// ---------------------------------------------------------------------
// Male shroud: a copy of the 176122-6 cavity so the OEM plug seats in it.
// Back wall z = 0..shroud_floor, mouth at the top. Print back wall down.
// ---------------------------------------------------------------------
module male_shroud(s, with_flange = true) {
    L = s_len(s);
    zf = shroud_floor;
    zt = sh_len(s);
    rl = s_rail(s);
    union() {
        difference() {
            union() {
                box_xy(-sh_w(s) / 2, sh_w(s) / 2, sh_y0(s), sh_y1(s), 0, zt);
                if (with_flange) box_xy(-fl_w(s) / 2, fl_w(s) / 2, fl_y0(s), fl_y1(s), -flange_t, 0);
            }
            box_xy(-L / 2, L / 2, 0, HDR_CAV_H, zf, zt + 1);
            for (sl = s_slots(s)) {
                box_xy(cx(s, sl[0]), cx(s, sl[1]), HDR_CAV_H - 0.01, HDR_SLOT_TOP, zf, zt + 1);
                hull() {                                    // 45° chamfer on the slot edges
                    box_xy(cx(s, sl[0]), cx(s, sl[1]), HDR_CAV_H + HDR_SLOT_CHAMF, HDR_CAV_H + HDR_SLOT_CHAMF + 0.01, zf, zt + 1);
                    box_xy(cx(s, sl[0]) - HDR_SLOT_CHAMF, cx(s, sl[1]) + HDR_SLOT_CHAMF,
                           HDR_CAV_H - 0.01, HDR_CAV_H, zf, zt + 1);
                }
            }
            for (g = s_grooves(s))
                box_xy(cx(s, g[0]), cx(s, g[1]), -HDR_GROOVE_D, 0.01, zf, zt + 1);
            translate([0, 0, zt]) mouth_chamfer(s);
            for_pins(s) tab_cavity($t, $out, zf, with_flange ? -flange_t : 0);
            if (with_flange)
                translate([fl_w(s) / 2 - 1.5, fl_y1(s) - 1.5, -flange_t - 0.6]) cylinder(d = 1.2, h = 1);   // pin 1 corner
        }
        // polarising rails on both end walls
        for (sx = [-1, 1])
            translate([sx > 0 ? L / 2 - rl[2] : -L / 2, rl[0], zf])
                cube([rl[2] + 0.01, rl[1] - rl[0], HDR_DEPTH - 1.0]);
        // lock bar at the mouth across the centre rib: the OEM plug's hooks snap behind it
        if (lock_bar)
            hull() {
                box_xy(-lock_bar_hw, lock_bar_hw, lock_bar_y0, HDR_SLOT_TOP + 0.01, zt - lock_bar_depth, zt - 0.5);
                box_xy(-lock_bar_hw, lock_bar_hw, lock_bar_y0 + 0.5, HDR_SLOT_TOP + 0.01, zt - lock_bar_depth, zt);
            }
    }
}

module mouth_chamfer(s) {
    c = 0.6;
    L = s_len(s);
    hull() {
        box_xy(-L / 2, L / 2, 0, HDR_CAV_H, -c, -c + 0.01);
        box_xy(-L / 2 - c, L / 2 + c, -c, HDR_CAV_H + c, 0, 0.01);
    }
}

// ---------------------------------------------------------------------
// Chamber: two screw-together halves clamping both flanges.
// Axis along Z: female plug end at z = 0, shroud end at z = Lc.
// Split plane y = fl_mid; half = 0 bottom, 1 top.
// ---------------------------------------------------------------------
function chamber_len_total() = 2 * chamber_wall + 2 * flange_t + chamber_len;
function fl_mid(s) = (fl_y0(s) + fl_y1(s)) / 2;
function ch_ow(s)  = fl_w(s) + 2 * (slide_clr + chamber_wall);
function ch_oy0(s) = fl_y0(s) - slide_clr - chamber_wall;
function ch_oy1(s) = fl_y1(s) + slide_clr + chamber_wall;

module chamber_shell(s) {
    Lc = chamber_len_total();
    ow = ch_ow(s); oy0 = ch_oy0(s); oy1 = ch_oy1(s);
    L = s_len(s);
    ear = screw_head_d + 2;
    difference() {
        union() {
            box_xy(-ow / 2, ow / 2, oy0, oy1, 0, Lc);
            for (sx = [-1, 1], z = [Lc * 0.25, Lc * 0.75])
                translate([sx * (ow / 2 + ear / 2 - 0.5) - ear / 2, oy0, z - ear / 2])
                    cube([ear, oy1 - oy0, ear]);
            // zip-tie anchor under the plug end: tie the interposer to the ECU bracket
            box_xy(-ziptie_w / 2 - 2, ziptie_w / 2 + 2, oy0 - 3, oy0 + 0.01, 0, 8);
        }
        box_xy(-fl_w(s) / 2 - slide_clr, fl_w(s) / 2 + slide_clr, fl_y0(s) - slide_clr, fl_y1(s) + slide_clr,
               chamber_wall, Lc - chamber_wall);
        // end opening for the female plug body (incl. key ridges)
        box_xy(-L / 2 - slide_clr, L / 2 + slide_clr, -slide_clr, HDR_SLOT_TOP + slide_clr, -1, chamber_wall + 0.01);
        // end opening for the male shroud body
        box_xy(-sh_w(s) / 2 - slide_clr, sh_w(s) / 2 + slide_clr, sh_y0(s) - slide_clr, sh_y1(s) + slide_clr,
               Lc - chamber_wall - 0.01, Lc + 1);
        // tap ribbon exit slot (+X wall)
        translate([ow / 2 - chamber_wall - 1, fl_mid(s) - tap_slot_h / 2, Lc / 2 - tap_slot_w / 2])
            cube([chamber_wall + 2, tap_slot_h, tap_slot_w]);
        // zip-tie slot
        translate([-ziptie_w / 2, oy0 - 2.2, -1]) cube([ziptie_w, 1.6, 10]);
        // screw holes through the ears (along Y)
        for (sx = [-1, 1], z = [Lc * 0.25, Lc * 0.75])
            translate([sx * (ow / 2 + ear / 2 - 0.5), oy0 - 1, z])
                rotate([-90, 0, 0]) cylinder(d = screw_d, h = oy1 - oy0 + 2);
    }
}

module chamber_half(s, half) {
    Lc = chamber_len_total();
    big = 400;
    ear = screw_head_d + 2;
    ow = ch_ow(s);
    oy1 = ch_oy1(s);
    difference() {
        intersection() {
            chamber_shell(s);
            if (half == 0) translate([-big / 2, -big + fl_mid(s), -1]) cube([big, big, Lc + 2]);
            else           translate([-big / 2, fl_mid(s), -1]) cube([big, big, Lc + 2]);
        }
        // top half: clearance holes + counterbores
        if (half == 1)
            for (sx = [-1, 1], z = [Lc * 0.25, Lc * 0.75])
                translate([sx * (ow / 2 + ear / 2 - 0.5), fl_mid(s) - 1, z])
                    rotate([-90, 0, 0]) {
                        cylinder(d = screw_d + 0.6, h = big);
                        translate([0, 0, oy1 - fl_mid(s) - 1.5]) cylinder(d = screw_head_d, h = big);
                    }
    }
}

// Print orientation: split face down
module chamber_half_print(s, half) {
    // rotate about X so the split plane (y = fl_mid) lands on z = 0, body above
    if (half == 0) rotate([-90, 0, 0]) translate([0, -fl_mid(s), 0]) chamber_half(s, 0);
    else           rotate([ 90, 0, 0]) translate([0, -fl_mid(s), 0]) chamber_half(s, 1);
}

// ---------------------------------------------------------------------
// Assembled view (for checking fit; not for printing)
// ---------------------------------------------------------------------
module section_assembly(s, explode = 0) {
    Lc = chamber_len_total();
    Lp = plug_len(s);
    color("SteelBlue") translate([0, 0, chamber_wall - Lp - explode]) female_plug(s);
    color("Orange")    translate([0, 0, Lc - chamber_wall + explode]) male_shroud(s);
    color("Gray", 0.35) chamber_half(s, 0);
    color("Gray", 0.35) translate([0, explode / 2, 0]) chamber_half(s, 1);
}

// ---------------------------------------------------------------------
// Tap box: houses the ESP32-S3 tap board, one ribbon inlet per section.
// ---------------------------------------------------------------------
function tapbox_in()  = [tapbox_pcb[0] + 2, tapbox_pcb[1] + 2];
function tapbox_h()   = 3 + tapbox_pcb[2] + tapbox_parts_h;

module tapbox_base() {
    iw = tapbox_in()[0]; id = tapbox_in()[1]; h = tapbox_h();
    w = tapbox_wall;
    difference() {
        translate([-w, -w, -w]) cube([iw + 2 * w, id + 2 * w, h + w]);
        cube([iw, id, h + 1]);
        for (i = [0 : tapbox_inlets - 1])
            translate([(i + 0.5) * iw / tapbox_inlets - tap_slot_w / 2, -w - 1, 3 + tapbox_pcb[2] + 1])
                cube([tap_slot_w, w + 2, tap_slot_h]);
        translate([iw - 1, id / 2 - 5, 3 + tapbox_pcb[2]]) cube([w + 2, 10, 4.5]);            // USB-C
        translate([-w - 1, id / 2, 3 + tapbox_pcb[2] + 3]) rotate([0, 90, 0]) cylinder(d = 6.5, h = w + 2); // CAN/sensor bus
    }
    for (x = [3.5, tapbox_pcb[0] - 2.5], y = [3.5, tapbox_pcb[1] - 2.5])
        translate([x, y, 0]) difference() {
            cylinder(d = 5, h = 3);
            cylinder(d = 2.1, h = 4);
        }
}

module tapbox_lid() {
    iw = tapbox_in()[0]; id = tapbox_in()[1];
    w = tapbox_wall;
    translate([-w, -w, 0]) cube([iw + 2 * w, id + 2 * w, 1.6]);
    difference() {
        translate([slide_clr, slide_clr, 1.59]) cube([iw - 2 * slide_clr, id - 2 * slide_clr, 2]);
        translate([1.2 + slide_clr, 1.2 + slide_clr, 1.5]) cube([iw - 2.4 - 2 * slide_clr, id - 2.4 - 2 * slide_clr, 3]);
    }
}
