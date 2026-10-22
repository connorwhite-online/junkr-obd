// =====================================================================
//  ECU interposer — parametric library
//
//  One "section" (A22 / B16 / C26) = four printed parts:
//
//    ECU header  <==  female_plug  [chamber_half x2]  male_shroud  <==  OEM harness plug
//                     (receptacle                     (male tabs,
//                      terminals)                      mimics ECU header)
//
//  Every pin passes straight through on a short pigtail inside the chamber.
//  Taps are spliced on in the chamber and leave through the side slot on a
//  ribbon to the tap box (ESP32 board). Nothing on the vehicle is cut.
//
//  Coordinates: connector face in XY, mating axis = Z.  Row 0 is +Y
//  (latch side).  Parts are modelled in print orientation where possible.
// =====================================================================

include <params.scad>
include <sections.scad>

// ---------------------------------------------------------------------
// Derived geometry
// ---------------------------------------------------------------------
function grid_w(s)  = s_cols(s) * s_px(s);
function grid_h(s)  = s_rows(s) * s_py(s);
function cell_x(s, c) = (c - (s_cols(s) - 1) / 2) * s_px(s);
function cell_y(s, r) = ((s_rows(s) - 1) / 2 - r) * s_py(s);

// Female plug outer (this is also the OEM harness-plug envelope the shroud must accept)
function plug_w(s)  = grid_w(s) + 2 * shell_wall;
function plug_h(s)  = grid_h(s) + 2 * shell_wall;

function max_body_len(s) = max([ for (row = s_map(s)) for (k = row) k == 0 ? 0 : body_len(term(k)) ]);
function plug_len(s) = face_wall + max_body_len(s) + plug_len_extra;

// Latch envelope above the plug's top face
function latch_top(s)  = plug_h(s) / 2 + latch_gap + latch_t;   // arm top (undeflected)
function hook_top(s)   = latch_top(s) + latch_hook;

// Shroud
function shroud_in_w(s)  = plug_w(s) + 2 * slide_clr;
function shroud_in_y0(s) = -plug_h(s) / 2 - slide_clr;
function shroud_in_y1(s) = latch_top(s) + slide_clr;
function shroud_out_w(s)  = shroud_in_w(s) + 2 * shell_wall;
function shroud_out_y0(s) = shroud_in_y0(s) - shell_wall;
function shroud_out_y1(s) = shroud_in_y1(s) + shell_wall;

// Common flange (both ends) that the chamber clamps
flange_t     = 2.0;
flange_extra = 1.6;
function fl_w(s)  = shroud_out_w(s) + 2 * flange_extra;
function fl_y0(s) = shroud_out_y0(s) - flange_extra;
function fl_y1(s) = shroud_out_y1(s) + flange_extra;

key_h = 0.8;   // key rib height

// Thinnest wall left between neighbouring cavities. OEM Multilock housings
// run ~0.3-0.5 mm here, which is why the terminal-holding parts want resin
// or MJF nylon rather than 0.4 mm-nozzle FDM.
function web_x(s) = min([ for (row = s_map(s)) for (k = row) if (k != 0) s_px(s) - body_w(term(k)) - 2 * clr ]);
function web_y(s) = s_rows(s) < 2 ? 99 : min([ for (row = s_map(s)) for (k = row) if (k != 0) s_py(s) - body_h(term(k)) - 2 * clr - lance_h(term(k)) ]);
module check_walls(s) {
    wx = web_x(s); wy = web_y(s);
    echo(str(s_name(s), ": cavity webs x = ", wx, " mm, y = ", wy == 99 ? "n/a" : wy, " mm (min_wall = ", min_wall, ")"));
    if (min(wx, wy) < min_wall)
        echo(str("WARNING: ", s_name(s), " webs below min_wall; use resin/MJF or adjust T040/T070 / clr"));
}

// ---------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------
module box_xy(w, y0, y1, z0, z1) {
    translate([-w / 2, y0, z0]) cube([w, y1 - y0, z1 - z0]);
}

module for_cells(s) {
    for (r = [0 : s_rows(s) - 1], c = [0 : s_cols(s) - 1]) {
        k = s_map(s)[r][c];
        if (k != 0)
            translate([cell_x(s, c), cell_y(s, r), 0])
                let ($k = k, $r = r, $c = c,
                     $out = (cell_y(s, r) >= 0) ? 1 : -1)   // lance faces outward
                    children();
    }
}

// Key ribs (positive) / key slots (negative, pass extra clearance)
module keys(s, y_top, y_bot, z0, z1, grow = 0) {
    for (k = s_keys(s)) {
        x = -plug_w(s) / 2 + k[0];
        w = k[1] + 2 * grow;
        if (k[2] == 0)
            translate([x - w / 2, y_top - 0.01, z0]) cube([w, key_h + grow + 0.01, z1 - z0]);
        else
            translate([x - w / 2, y_bot - key_h - grow, z0]) cube([w, key_h + grow + 0.01, z1 - z0]);
    }
}

// ---------------------------------------------------------------------
// Terminal cavities
// ---------------------------------------------------------------------

// Receptacle (female) cavity. Front face at z = 0, terminal inserted from +Z.
module receptacle_cavity(t, out, length) {
    bw = body_w(t) + 2 * clr;
    bh = body_h(t) + 2 * clr;
    // tab entry through the front wall, with a 45° lead-in
    tw = tab_w(t) + 2 * clr + 0.2;
    tt = tab_t(t) + 2 * clr + 0.3;
    translate([-tw / 2, -tt / 2, -1]) cube([tw, tt, face_wall + 2]);
    hull() {
        translate([-tw / 2, -tt / 2, 0.4]) cube([tw, tt, 0.01]);
        translate([-tw / 2 - 0.4, -tt / 2 - 0.4, -0.01]) cube([tw + 0.8, tt + 0.8, 0.01]);
    }
    // terminal body
    translate([-bw / 2, -bh / 2, face_wall]) cube([bw, bh, length]);
    // lance relief: the terminal's spring lance pops out here; the step at
    // z = face_wall + lance_from_front is what stops it backing out
    lh = lance_h(t);
    translate([-bw / 2 + 0.2, out > 0 ? bh / 2 - 0.01 : -bh / 2 - lh, face_wall])
        cube([bw - 0.4, lh + 0.01, lance_ff(t)]);
}

// Male tab cavity. Floor top at z = top, tab pokes out of the floor top,
// terminal inserted from below (-Z).
module tab_cavity(t, out, top, bottom) {
    bw = body_w(t) + 2 * clr;
    bh = body_h(t) + 2 * clr;
    tw = tab_w(t) + 2 * clr;
    tt = tab_t(t) + 2 * clr;
    web = 1.0;                                      // floor web the tab shoulder seats on
    translate([-tw / 2, -tt / 2, top - web - 0.01]) cube([tw, tt, web + 1]);
    translate([-bw / 2, -bh / 2, bottom - 1]) cube([bw, bh, top - web - bottom + 1]);
    lh = lance_h(t);
    translate([-bw / 2 + 0.2, out > 0 ? bh / 2 - 0.01 : -bh / 2 - lh, top - web - lance_ff(t)])
        cube([bw - 0.4, lh + 0.01, lance_ff(t)]);
}

// ---------------------------------------------------------------------
// Female plug — mates with the ECU header. Print front face down.
// ---------------------------------------------------------------------
module female_plug(s, with_flange = true) {
    L = plug_len(s);
    W = plug_w(s);
    H = plug_h(s);
    check_walls(s);
    difference() {
        union() {
            box_xy(W, -H / 2, H / 2, 0, L);
            keys(s, H / 2, -H / 2, 0, L - 3);
            latch_arm(s);
            if (with_flange) box_xy(fl_w(s), fl_y0(s), fl_y1(s), L, L + flange_t);
        }
        for_cells(s) receptacle_cavity(term($k), $out, L + flange_t + 2);
        // pin-1 marker on the rear flange
        if (with_flange)
            translate([-fl_w(s) / 2 + 1.5, fl_y1(s) - 1.5, L + flange_t - 0.4]) cylinder(d = 1.2, h = 1);
    }
}

// Cantilever latch on the plug's top face. Hook near the front, root toward the rear.
module latch_arm(s) {
    H = plug_h(s);
    y_arm = H / 2 + latch_gap;
    // root block bridging the flex gap
    translate([-latch_w / 2, H / 2 - 0.01, latch_len]) cube([latch_w, latch_gap + latch_t + 0.01, 2.0]);
    // arm
    translate([-latch_w / 2, y_arm, 1.0]) cube([latch_w, latch_t, latch_len + 1.0]);
    // hook: ramps up from the front (leading edge), square back face locks
    translate([-latch_w / 2, y_arm + latch_t - 0.01, 0])
        rotate([90, 0, 90])
            linear_extrude(latch_w)
                polygon([[1.0, 0], [2.6, latch_hook], [2.6, 0]]);
}

// ---------------------------------------------------------------------
// Male shroud — mimics the ECU header so the OEM plug latches into it.
// Floor at z = 0..shroud_floor, mouth at the top. Print floor down.
// ---------------------------------------------------------------------
module male_shroud(s, with_flange = true) {
    zf = shroud_floor;
    zt = shroud_floor + shroud_depth;
    difference() {
        union() {
            box_xy(shroud_out_w(s), shroud_out_y0(s), shroud_out_y1(s), 0, zt);
            if (with_flange) box_xy(fl_w(s), fl_y0(s), fl_y1(s), -flange_t, 0);
        }
        // pocket the OEM plug slides into
        box_xy(shroud_in_w(s), shroud_in_y0(s), shroud_in_y1(s), zf, zt + 1);
        // key slots matching the plug's ribs
        keys(s, plug_h(s) / 2 + slide_clr, -plug_h(s) / 2 - slide_clr, zf, zt + 1, grow = slide_clr);
        // latch window: plug hook (front 1.0..2.6 mm) pops up into it when seated
        translate([-(latch_w + 0.6) / 2, shroud_in_y1(s) - 0.5, zf + 0.8])
            cube([latch_w + 0.6, shell_wall + 1, 2.2]);
        // mouth chamfer
        translate([0, 0, zt]) mouth_chamfer(s);
        // male tab cavities
        for_cells(s) tab_cavity(term($k), $out, zf, with_flange ? -flange_t : 0);
        // pin-1 marker on the flange underside
        if (with_flange)
            translate([-fl_w(s) / 2 + 1.5, fl_y1(s) - 1.5, -flange_t - 0.6]) cylinder(d = 1.2, h = 1);
    }
}

module mouth_chamfer(s) {
    c = 0.8;
    hull() {
        box_xy(shroud_in_w(s), shroud_in_y0(s), shroud_in_y1(s), -c, -c + 0.01);
        box_xy(shroud_in_w(s) + 2 * c, shroud_in_y0(s) - c, shroud_in_y1(s) + c, 0, 0.01);
    }
}

// ---------------------------------------------------------------------
// Chamber — two screw-together halves clamping both flanges.
// Axis along Z: female plug end at z = 0, shroud end at z = Lc.
// Split plane y = fl_mid; half = 0 bottom, 1 top.
// ---------------------------------------------------------------------
function chamber_len_total() = 2 * chamber_wall + 2 * flange_t + chamber_len;
function fl_mid(s) = (fl_y0(s) + fl_y1(s)) / 2;

module chamber_shell(s) {
    Lc = chamber_len_total();
    ow = fl_w(s) + 2 * (slide_clr + chamber_wall);
    oy0 = fl_y0(s) - slide_clr - chamber_wall;
    oy1 = fl_y1(s) + slide_clr + chamber_wall;
    ear = screw_head_d + 2;
    difference() {
        union() {
            box_xy(ow, oy0, oy1, 0, Lc);
            for (sx = [-1, 1], z = [Lc * 0.25, Lc * 0.75])
                translate([sx * (ow / 2 + ear / 2 - 0.5) - ear / 2, oy0, z - ear / 2])
                    cube([ear, oy1 - oy0, ear]);
        }
        // inner cavity (flanges + wire space)
        box_xy(fl_w(s) + 2 * slide_clr, fl_y0(s) - slide_clr, fl_y1(s) + slide_clr,
               chamber_wall, Lc - chamber_wall);
        // end opening for the female plug body (incl. latch envelope)
        box_xy(plug_w(s) + 2 * slide_clr, -plug_h(s) / 2 - key_h - slide_clr, hook_top(s) + slide_clr, -1, chamber_wall + 0.01);
        // end opening for the male shroud body
        box_xy(shroud_out_w(s) + 2 * slide_clr, shroud_out_y0(s) - slide_clr, shroud_out_y1(s) + slide_clr,
               Lc - chamber_wall - 0.01, Lc + 1);
        // tap ribbon exit slot (+X wall)
        translate([ow / 2 - chamber_wall - 1, fl_mid(s) - tap_slot_h / 2, Lc / 2 - tap_slot_w / 2])
            cube([chamber_wall + 2, tap_slot_h, tap_slot_w]);
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
    ow = fl_w(s) + 2 * (slide_clr + chamber_wall);
    oy1 = fl_y1(s) + slide_clr + chamber_wall;
    difference() {
        intersection() {
            chamber_shell(s);
            if (half == 0) translate([-big / 2, -big + fl_mid(s), -1]) cube([big, big, Lc + 2]);
            else           translate([-big / 2, fl_mid(s), -1]) cube([big, big, Lc + 2]);
        }
        // top half: open up the screw holes to clearance + counterbore
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
    if (half == 0) rotate([90, 0, 0]) translate([0, -fl_mid(s), 0]) chamber_half(s, 0);
    else           rotate([-90, 0, 0]) translate([0, -fl_mid(s), 0]) chamber_half(s, 1);
}

// ---------------------------------------------------------------------
// Assembled view (for checking fit; not for printing)
// ---------------------------------------------------------------------
module section_assembly(s, explode = 0) {
    Lc = chamber_len_total();
    L = plug_len(s);
    // female plug: flange inside the chamber at the z = 0 end, body pokes out toward -Z
    color("SteelBlue") translate([0, 0, chamber_wall - L - explode]) female_plug(s);
    // male shroud: flange inside at the z = Lc end, mouth facing +Z
    color("Orange") translate([0, 0, Lc - chamber_wall + explode]) male_shroud(s);
    color("Gray", 0.35) chamber_half(s, 0);
    color("Gray", 0.35) translate([0, explode / 2, 0]) chamber_half(s, 1);
}

// ---------------------------------------------------------------------
// Tap box — houses the ESP32-S3 tap board, one ribbon inlet per section.
// ---------------------------------------------------------------------
function tapbox_in()  = [tapbox_pcb[0] + 2, tapbox_pcb[1] + 2];
function tapbox_h()   = 3 + tapbox_pcb[2] + tapbox_parts_h;

module tapbox_base() {
    iw = tapbox_in()[0]; id = tapbox_in()[1]; h = tapbox_h();
    w = tapbox_wall;
    difference() {
        translate([-w, -w, -w]) cube([iw + 2 * w, id + 2 * w, h + w]);
        cube([iw, id, h + 1]);
        // ribbon inlets on the long (-Y) wall
        for (i = [0 : tapbox_inlets - 1])
            translate([(i + 0.5) * iw / tapbox_inlets - tap_slot_w / 2, -w - 1, 3 + tapbox_pcb[2] + 1])
                cube([tap_slot_w, w + 2, tap_slot_h]);
        // USB-C service port on the short (+X) wall
        translate([iw - 1, id / 2 - 5, 3 + tapbox_pcb[2]]) cube([w + 2, 10, 4.5]);
        // 12 V / CAN pigtail grommet on the -X wall
        translate([-w - 1, id / 2, 3 + tapbox_pcb[2] + 3]) rotate([0, 90, 0]) cylinder(d = 6.5, h = w + 2);
    }
    // PCB standoffs (M2.5 self-tap)
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
    // locating lip
    difference() {
        translate([slide_clr, slide_clr, 1.59]) cube([iw - 2 * slide_clr, id - 2 * slide_clr, 2]);
        translate([1.2 + slide_clr, 1.2 + slide_clr, 1.5]) cube([iw - 2.4 - 2 * slide_clr, id - 2.4 - 2 * slide_clr, 3]);
    }
}
