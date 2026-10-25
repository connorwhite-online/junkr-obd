# Gauge fonts

LVGL bitmap fonts cut from **Inter** (SIL Open Font License 1.1,
https://rsms.me/inter/), 4 bpp, with `lv_font_conv` 1.5.2.

| File | Face | px | Glyphs |
| --- | --- | --- | --- |
| `inter_bold_104.c` | Bold | 104 | `-./0-9` (big boost / rpm / alert value) |
| `inter_bold_60.c` | Bold | 60 | `-./0-9` (EGT) |
| `inter_bold_32.c` | Bold | 32 | ASCII + `° · → λ` |
| `inter_semi_19.c` | SemiBold | 19 | ASCII + `° · → λ` |
| `inter_bold_15.c` | Bold | 15 | ASCII + `° · → λ` |
| `inter_bold_28.c` | Bold | 28 | `-./0-9°` (Drive page readouts) |
| `icons_26.c` | Material Icons Round (Apache 2.0) | 26 | U+EF55 flame, U+EFD8 air, U+F076 thermostat |

Regenerate (example for one size):

```bash
npx lv_font_conv@1.5.2 --no-compress --bpp 4 --format lvgl --lv-include lvgl.h \
  --font Inter-Bold.otf -r 0x20-0x7E -r 0xB0,0xB7,0x2192,0x3BB \
  --size 32 -o inter_bold_32.c --force-fast-kern-format
```

The icon font is `material-icons-round.woff2` from
`@material-design-icons/font` 0.14.15, converted to TTF with fontTools.

A glyph that isn't in the range just doesn't draw, so add it to `-r`
before using it in `ui.cpp`.
