#!/usr/bin/env python3
"""Board images for the docs: top (F.Cu + silk), and all copper layers.

    /usr/bin/python3 gen_renders.py   ->  ../renders/board_top.png, board_copper.png
"""
import os
import subprocess
import tempfile

import cairosvg

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.join(HERE, "..")
BOARD = os.environ.get("FAB_BOARD", os.path.join(ROOT, "junkr_ecu.kicad_pcb"))
OUT = os.path.join(ROOT, "renders")

VIEWS = {
    "board_top": "F.Cu,F.Silkscreen,Edge.Cuts",
    "board_copper": "F.Cu,In2.Cu,In3.Cu,In4.Cu,B.Cu,Edge.Cuts",
}


def main():
    os.makedirs(OUT, exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        for name, layers in VIEWS.items():
            svg = os.path.join(tmp, name + ".svg")
            subprocess.check_call(["kicad-cli", "pcb", "export", "svg", "-o", svg, "-l", layers,
                                   "--page-size-mode", "2", "--exclude-drawing-sheet", BOARD],
                                  stdout=subprocess.DEVNULL)
            png = os.path.join(OUT, name + ".png")
            cairosvg.svg2png(url=svg, write_to=png, output_width=2400, background_color="#10131a")
            print(png)


if __name__ == "__main__":
    main()
