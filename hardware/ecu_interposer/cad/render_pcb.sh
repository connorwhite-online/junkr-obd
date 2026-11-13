#!/bin/sh
# Renders and STLs for the PCB interposer (needs OpenSCAD >= 2021 and xvfb-run).
# Run kicad/interposer/export_scene.py first if the board changed.
set -e
cd "$(dirname "$0")"
R=../../../docs/pcb-interposer
mkdir -p "$R"
O="--colorscheme=Tomorrow"
xvfb-run -a openscad $O -o "$R/assembly_ecu_side.png"     -D 'part="assembly"' --imgsize=1800,1100 --camera=-75,-12,8,58,0,25,330 pcb_interposer.scad
xvfb-run -a openscad $O -o "$R/assembly_harness_side.png" -D 'part="assembly"' --imgsize=1800,1100 --camera=-75,10,8,58,0,205,330 pcb_interposer.scad
xvfb-run -a openscad $O -o "$R/exploded.png"              -D 'part="exploded"' --imgsize=1800,1300 --camera=-75,0,8,62,0,25,370 pcb_interposer.scad
xvfb-run -a openscad $O -o "$R/board_ecu_side.png"        -D 'part="board"'    --imgsize=1800,1000 --camera=-75,-5,6,58,0,25,290 pcb_interposer.scad
xvfb-run -a openscad $O -o "$R/side.png"                  -D 'part="assembly"' --imgsize=1800,700 --projection=o --camera=-75,-8,10,90,0,270,210 pcb_interposer.scad
openscad -o ../stl/pcb_tray.stl -D 'part="tray"' pcb_interposer.scad
openscad -o ../stl/pcb_lid.stl  -D 'part="lid"'  pcb_interposer.scad
