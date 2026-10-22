// =====================================================================
//  ECU interposer — top level. Pick what to render with `part` / `sec`.
//
//  openscad -o ../stl/C26_female_plug.stl -D 'part="female_plug"' -D 'sec_i=2' main.scad
//  (or just run ./export_all.sh)
//
//  part: "assembly" | "female_plug" | "male_shroud" | "chamber_bottom"
//        | "chamber_top" | "tapbox_base" | "tapbox_lid"
//  sec_i: 0 = A22, 1 = B16, 2 = C26
// =====================================================================

include <interposer.scad>

part    = "assembly";
sec_i   = 2;
explode = 12;

s = SECTIONS[sec_i];

if (part == "assembly")            section_assembly(s, explode);
else if (part == "female_plug")    female_plug(s);                         // front face down
else if (part == "male_shroud")    translate([0, 0, flange_t]) male_shroud(s); // flange down
else if (part == "chamber_bottom") chamber_half_print(s, 0);
else if (part == "chamber_top")    chamber_half_print(s, 1);
else if (part == "tapbox_base")    tapbox_base();
else if (part == "tapbox_lid")     tapbox_lid();
