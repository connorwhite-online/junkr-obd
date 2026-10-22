// =====================================================================
//  Diagram scene: one section with every terminal and pigtail, for the
//  exploded view and the cross-sections in ../diagram/. Not for printing.
//
//  view = "exploded" | "assembled"  (3D, coloured)
//         "cut"                     (2D section of one component, for SVG export)
//  comp = component to emit in "cut" view (see comps below), "" = all
//  cut  = "x" (across the mating axis at z = cut_at)
//         "l" (lengthwise, plane x = cut_at, through a pin column)
// =====================================================================

include <interposer.scad>

view   = "exploded";
sec_i  = 0;
comp   = "";
cut    = "x";
cut_at = 4.0;
E      = 30;            // explode step

s = SECTIONS[sec_i];

// ---- terminal models (envelopes; body sizes are [MEASURE]) ----------
function wire_d(t) = t == T070 ? 2.2 : 1.6;

module receptacle_model(t) {      // mating end at z = 0, wire out +Z
    bw = body_w(t); bh = body_h(t); L = body_len(t) - 4;
    difference() {
        translate([-bw / 2, -bh / 2, 0]) cube([bw, bh, L]);
        translate([-bw / 2 + 0.25, -bh / 2 + 0.25, -0.01]) cube([bw - 0.5, bh - 0.5, L - 1]);
    }
    // contact spring folded inside the box
    translate([-tab_w(t) / 2 - 0.1, -bh / 2 + 0.25, 1]) cube([tab_w(t) + 0.2, (bh - tab_t(t)) / 2 - 0.3, L - 3]);
    translate([-bw * 0.35, -bh * 0.35, L]) cube([bw * 0.7, bh * 0.7, 2.2]);          // wire crimp
    translate([0, 0, L + 2.2]) cylinder(d = wire_d(t) + 0.3, h = 1.8);              // insulation crimp
}

module tab_model(t, proud) {      // floor top at z = 0, tab up, wire out -Z
    translate([-tab_w(t) / 2, -tab_t(t) / 2, -1.01]) cube([tab_w(t), tab_t(t), proud + 1.01]);
    translate([-tab_w(t) / 2, -tab_t(t) / 2, proud - 0.4])                           // chamfered tip
        cube([tab_w(t), tab_t(t), 0.4]);
    bl = male_body_len(t);
    translate([-body_w(t) / 2 + 0.1, -body_h(t) / 2 + 0.1, -1 - bl]) cube([body_w(t) - 0.2, body_h(t) - 0.2, bl]);
    translate([-body_w(t) * 0.35, -body_h(t) * 0.35, -1 - bl - 2.2]) cube([body_w(t) * 0.7, body_h(t) * 0.7, 2.2]);
    translate([0, 0, -1 - bl - 4]) cylinder(d = wire_d(t) + 0.3, h = 1.8);
}

module header_pin(t, proud) {     // PCB pin of the ECU header
    translate([-tab_w(t) / 2, -tab_t(t) / 2, -6]) cube([tab_w(t), tab_t(t), proud + 6]);
}

module wire(p0, p1, d) { hull() { translate(p0) sphere(d = d, $fn = 12); translate(p1) sphere(d = d, $fn = 12); } }

// ---- assembled positions (z) ----------------------------------------
Lp   = plug_len(s);
Lc   = chamber_len_total();
z_pl = 0.2;                                   // female plug front face (header floor = 0)
z_sh = z_pl + Lp + Lc - 2 * chamber_wall;     // male shroud base (flange top)
z_fl = z_sh + shroud_floor;                   // shroud floor top
z_oem = z_fl + 0.2;                           // OEM plug front face
rec_z0 = face_wall;                           // receptacle mating end, plug coords
rec_top = body_len(T040) - 4 + 4;             // receptacle length incl. crimps

function ex(k) = view == "exploded" ? k * E : 0;
// explode offsets per layer
O_HDR = -ex(1); O_PLUG = 0; O_REC = ex(1); O_TAB = ex(2.2); O_SH = ex(3.2); O_OEM = ex(4.4); O_OREC = ex(5.4);

wire_cols = ["#c0392b", "#222222", "#2e86c1", "#27ae60", "#f1c40f", "#8e44ad", "#e67e22", "#7f8c8d", "#ecf0f1"];

module comp_on(name) { if (comp == "" || comp == name) children(); }

module scene() {
    comp_on("header")     color("#5d6d7e") translate([0, 0, O_HDR - shroud_floor]) male_shroud(s, with_flange = false);
    comp_on("header_pins") color("#d4ac0d") translate([0, 0, O_HDR]) for_pins(s) header_pin($t, tab_proud($code));
    comp_on("plug")       color("#2e86c1") translate([0, 0, O_PLUG + z_pl]) female_plug(s);
    comp_on("receptacles") color("#ba7a45") translate([0, 0, O_REC + z_pl + rec_z0]) for_pins(s) receptacle_model($t);
    comp_on("tabs")       color("#d4ac0d") translate([0, 0, O_TAB + z_fl]) for_pins(s) tab_model($t, tab_proud($code));
    comp_on("shroud")     color("#e67e22") translate([0, 0, O_SH + z_sh]) male_shroud(s);
    comp_on("oem_plug")   color("#aab7b8", 0.55) translate([0, 0, O_OEM + z_oem]) female_plug(s, with_flange = false);
    comp_on("oem_receptacles") color("#ba7a45") translate([0, 0, O_OREC + z_oem + rec_z0]) for_pins(s) receptacle_model($t);
    if (view == "cut") comp_on("chamber")
        translate([0, 0, z_pl - chamber_wall + Lp]) { chamber_half(s, 0); chamber_half(s, 1); }
    comp_on("wires") {
        // pigtails: receptacle crimp -> tab crimp (one colour per column)
        for (i = [0 : len(s_cols(s)) - 1], r = [0, 1]) {
            col = s_cols(s)[i]; t = term(col[1]);
            p = [cx(s, col[0]), row_y(col[1], r)];
            z0 = O_REC + z_pl + rec_z0 + rec_top;
            z1 = O_TAB + z_fl - 1 - male_body_len(t) - 4;
            color(wire_cols[(2 * i + r) % len(wire_cols)])
                wire([p[0], p[1], z0], [p[0], p[1], z1], wire_d(t));
            // OEM harness wire stubs
            z2 = O_OREC + z_oem + rec_z0 + rec_top;
            color(wire_cols[(2 * i + r) % len(wire_cols)])
                wire([p[0], p[1], z2], [p[0], p[1], z2 + 10], wire_d(t));
        }
    }
}

ORIENT = [[0, 0, 1, 0], [1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 0, 1]];
MARKERS = [[-30, -10, -40], [30, -10, -40], [-30, 25, 200], [30, 25, 60], [0, 0, 120], [20, -5, 250]];

if (view == "cut") {
    if (cut == "x") projection(cut = true) translate([0, 0, -cut_at]) scene();
    else            projection(cut = true) rotate([0, 90, 0]) translate([-cut_at, 0, 0]) scene();
} else if (view == "markers") {
    multmatrix(ORIENT) for (p = MARKERS) translate(p) sphere(r = 1.5);
} else {
    multmatrix(ORIENT) scene();      // mating axis -> X, pin rows -> Y, height -> Z
}
