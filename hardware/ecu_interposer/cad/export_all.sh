#!/usr/bin/env bash
# Export every printable part for every section to ../stl/
set -euo pipefail
cd "$(dirname "$0")"
OUT=../stl
mkdir -p "$OUT"
names=(E5_26P E6_16P E8_22P)
for i in 0 1 2; do
  for p in female_plug male_shroud chamber_bottom chamber_top; do
    echo "-> ${names[$i]}_$p"
    openscad -q -o "$OUT/${names[$i]}_$p.stl" -D "part=\"$p\"" -D "sec_i=$i" main.scad
  done
  # fit-test plug without lock tines: safe to push into the ECU header in any material
  echo "-> ${names[$i]}_female_plug_fittest_no_latch"
  openscad -q -o "$OUT/${names[$i]}_female_plug_fittest_no_latch.stl" -D 'part="female_plug"' -D "sec_i=$i" -D latch_tines=false main.scad
done
for p in tapbox_base tapbox_lid; do
  echo "-> $p"
  openscad -q -o "$OUT/$p.stl" -D "part=\"$p\"" main.scad
done
echo "-> fit_coupon"
openscad -q -o "$OUT/fit_coupon.stl" fit_coupon.scad
