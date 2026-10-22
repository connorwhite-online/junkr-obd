#!/usr/bin/env bash
# Export every printable part for every section to ../stl/
set -euo pipefail
cd "$(dirname "$0")"
OUT=../stl
mkdir -p "$OUT"
names=(A22 B16 C26)
for i in 0 1 2; do
  for p in female_plug male_shroud chamber_bottom chamber_top; do
    echo "-> ${names[$i]}_$p"
    openscad -q -o "$OUT/${names[$i]}_$p.stl" -D "part=\"$p\"" -D "sec_i=$i" main.scad
  done
done
for p in tapbox_base tapbox_lid; do
  echo "-> $p"
  openscad -q -o "$OUT/$p.stl" -D "part=\"$p\"" main.scad
done
echo "-> fit_coupon"
openscad -q -o "$OUT/fit_coupon.stl" fit_coupon.scad
