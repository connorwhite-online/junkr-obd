#!/usr/bin/env bash
# Export every printable part to ../stl/ and the preview renders to ../renders/
set -euo pipefail
cd "$(dirname "$0")"
OUT=../stl
mkdir -p "$OUT" ../renders
for p in faceplate switch_plate rear_cover fit_frame cap_test sleeve; do
  echo "-> $p"
  openscad -q -o "$OUT/$p.stl" -D "part=\"$p\"" panel.scad
done
# one cap per button, named by its legend
tmp=$(mktemp --suffix=.echo)
openscad -o "$tmp" -D 'part="count"' panel.scad 2>/dev/null
n=$(sed -n 's/^ECHO: n_buttons = //p' "$tmp"); rm -f "$tmp"
for i in $(seq 0 $((n - 1))); do
  echo "-> cap $i"
  openscad -q -o "$OUT/cap_$i.stl" -D 'part="cap"' -D "cap_i=$i" panel.scad
done
# previews (needs a display; xvfb-run works headless)
run=""; [ -z "${DISPLAY:-}" ] && command -v xvfb-run >/dev/null && run="xvfb-run -a"
$run openscad -q -o ../renders/assembly.png --imgsize=2600,1300 --camera=0,-40,0,62,0,18,430 --colorscheme=Tomorrow panel.scad
$run openscad -q -o ../renders/assembly_back.png --imgsize=1600,900 --camera=0,0,-20,235,0,160,430 --colorscheme=Tomorrow panel.scad
$run openscad -q -o ../renders/front.png --imgsize=2400,840 --projection=o --camera=0,0,0,0,0,0,330 --colorscheme=Tomorrow panel.scad
$run openscad -q -o ../renders/exploded.png --imgsize=2200,1500 -D 'part="exploded"' --camera=0,0,-25,60,0,28,600 --colorscheme=Tomorrow panel.scad
# trim the empty background (needs Pillow; skipped without it)
python3 - ../renders/*.png <<'PY' || true
import sys
from PIL import Image, ImageChops
for f in sys.argv[1:]:
    im = Image.open(f).convert("RGB")
    bg = Image.new("RGB", im.size, im.getpixel((0, 0)))
    box = ImageChops.difference(im, bg).getbbox()
    if box:
        pad = 24
        box = (max(box[0] - pad, 0), max(box[1] - pad, 0), min(box[2] + pad, im.width), min(box[3] + pad, im.height))
        im.crop(box).save(f)
PY
