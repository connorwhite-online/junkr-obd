# Ordering the parts (rev C)

Nothing has been ordered. Stock and prices were checked on 2026-11-13.

## 1. The ECU header (J1): Ballenger Motorsports

TE 176122-6 can't be bought in small quantities: Digi-Key, Newark and Arrow
all start at 1,600 pieces with an 18-week lead time. The same 64-way header
is used on the NA Miata and OBD1 Honda ECUs, and Ballenger sells it singly:

- [NA Miata 64 Way ECU Header Receptacle, CONN-100747](https://www.bmotorsports.com/shop/product_info.php/products_id/3454): $29.99, in stock.

TE makes this header in -1, -6 and -7 colour versions (rusEFI wiki). Before
buying, ask Ballenger which TE number theirs is, and compare it with your
truck: its harness plugs must click into it.

## 2. Plugs and terminals: Digi-Key (`digikey_cart.csv`)

Upload `digikey_cart.csv` in Digi-Key's BOM Manager. The plug housings are
TE's 17451x-6 numbers, which rusEFI lists as the same mating plugs for this
header as 17510x-6 (those aren't stocked anywhere now).

| Ref | Part | Digi-Key stock | Each |
| --- | --- | --- | --- |
| J2 E8 22P | 174515-6 | 0 at Digi-Key; 4,205 on its Marketplace | $2.74 |
| J3 E6 16P | 174514-6 | 44 | $1.85 |
| J4 E5 26P | 174516-6 | 138 | $2.81 |
| 040 receptacle | 173716-1 (replaces 175061-1, which needs 32,000) | 63,638 | $0.16 |
| 070 receptacle | 173631-1 (cut tape) | 29,320 | $0.19 |

All three plugs are gray (-6). Read the part number moulded on each of
your truck's harness plugs first. If they end in -6 these match; if not,
tell me and I'll find the matching suffix.

## Backup for the plugs: Ballenger kits (terminals included)

- [22 Way, CONN-75908](https://www.bmotorsports.com/shop/product_info.php/products_id/3461): $12.25, 17 small + 7 large terminals.
- [16 Way, CONN-75907](https://www.bmotorsports.com/shop/product_info.php/products_id/3460): $11.25, 17 small terminals.
- [26 Way, CONN-75909](https://www.bmotorsports.com/shop/product_info.php/products_id/3462): $12.25, 17 small + 11 large terminals.

These are Miata parts, so the same key/colour check applies.

## Not from Digi-Key

- PCB: zip `gerbers/`, JLCPCB or PCBWay, 2 layers, 1.6 mm, 2 oz, HASL (~$15-30 for 5).
- 22 AWG wire 10 m, 18 AWG wire 3 m, 1 m braided sleeve, M2 x 8 screws (x4) and M2 x 20 (x1), USB-C car charger and cable, open-barrel crimper: Amazon or similar.
- Tray and lid: print at home in PETG, or SLS PA12 from the Materialize project (~$41).
