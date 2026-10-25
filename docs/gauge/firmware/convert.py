"""Turn the host-rendered gauge frames into round-masked PNGs.

    pio run -e gauge-preview -t exec
    python3 docs/gauge/firmware/convert.py

Reads .pio/gauge-preview/*.ppm (written by src/preview/main.cpp, the real
ui.cpp running on the PC) and writes one PNG per page next to this file,
plus contact.png with all of them.
"""
from pathlib import Path

from PIL import Image, ImageDraw

HERE = Path(__file__).resolve().parent
SRC = HERE.parents[2] / ".pio" / "gauge-preview"
BEZEL = (217, 217, 217)


def round_mask(img):
    w, h = img.size
    mask = Image.new("L", (w, h), 0)
    ImageDraw.Draw(mask).ellipse((0, 0, w - 1, h - 1), fill=255)
    out = Image.new("RGB", (w, h), BEZEL)
    out.paste(img, (0, 0), mask)
    return out


def main():
    frames = sorted(SRC.glob("*.ppm"))
    if not frames:
        raise SystemExit(f"no frames in {SRC}; run `pio run -e gauge-preview -t exec` first")
    shots = []
    for p in frames:
        img = round_mask(Image.open(p).convert("RGB"))
        img.save(HERE / f"{p.stem}.png", optimize=True)
        shots.append(img)
        print("wrote", HERE / f"{p.stem}.png")
    cols = 3
    rows = (len(shots) + cols - 1) // cols
    sheet = Image.new("RGB", (cols * 490 + 10, rows * 490 + 10), BEZEL)
    for i, img in enumerate(shots):
        sheet.paste(img, (10 + (i % cols) * 490, 10 + (i // cols) * 490))
    sheet.save(HERE / "contact.png", optimize=True)
    print("wrote", HERE / "contact.png")


if __name__ == "__main__":
    main()
