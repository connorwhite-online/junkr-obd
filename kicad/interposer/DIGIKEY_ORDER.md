# Digi-Key order (rev C)

Upload `digikey_cart.csv` at digikey.com > BOM Manager (or "Upload a list"
on the cart page). It maps Quantity, Manufacturer Part Number and Customer
Reference. Nothing has been ordered.

## Parts that need a check before you pay

Stock below is from distributor search results on 2026-11-13, not live.

| Part | What I found | If Digi-Key can't supply it |
| --- | --- | --- |
| 176122-6 header | No Digi-Key listing found. Others list ~$10-14; Avnet shows 0 stock / long lead. | Mouser, TTI, Master Electronics, or a pulled Toyota ECU header. |
| 175110-6, 175109-6 plugs | No Digi-Key listing found. | Mouser, TTI, automotiveconnectors.com. |
| 175111-6 plug (E5) | Listed at Digi-Key as non-stock, quote only. | 174516-6 (26P, stocked) may fit, but its key is unverified: compare with your E5 plug first. |
| 175061-1, 173631-1 receptacles | Not confirmed at Digi-Key; TTI sells 173631-1 on reels. | Mouser or TTI, often cut-tape or bag quantities. |
| ESP32-S3-DevKitC-1 | N16 ~$13 at Digi-Key; N16R8 not found. | N16, N8R8 and N16R8 share the footprint. Amazon also works. |

Read the part number moulded on each of your three harness plugs first:
the last digit of the housing sets its key and colour.

## Not from Digi-Key

- PCB: zip `gerbers/`, JLCPCB or PCBWay, 2 layers, 1.6 mm, 2 oz, HASL (~$15-30 for 5).
- 22 AWG wire 10 m, 18 AWG wire 3 m, 1 m braided sleeve, M2 x 8 screws (x4) and M2 x 20 (x1), USB-C car charger and cable, open-barrel crimper: Amazon or similar.
- Tray and lid: print at home in PETG, or SLS PA12 from the Materialize project (~$41).
