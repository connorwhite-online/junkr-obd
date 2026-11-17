S="../../../hardware/ecu_interposer/stl/";
module p(n,c,x,y,rz=0){ translate([x,y,0]) rotate([0,0,rz]) color(c) import(str(S,n,".stl")); }
A="#c9a227"; G="#4a4f57";
p("E6_16P_female_plug",A,0,0); p("E6_16P_male_shroud",A,45,0);
p("E8_22P_female_plug",A,0,45); p("E8_22P_male_shroud",A,50,45);
p("E6_16P_chamber_bottom",G,100,0); p("E6_16P_chamber_top",G,160,0);
p("E8_22P_chamber_bottom",G,100,50); p("E8_22P_chamber_top",G,160,50);
p("tapbox_min_base",G,0,110); p("tapbox_min_lid",G,120,110);
