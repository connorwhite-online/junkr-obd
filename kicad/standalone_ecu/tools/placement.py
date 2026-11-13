"""Where every part goes (board mm, origin top-left, y down).

Fixed anchors are listed explicitly. Everything else is placed by
gen_pcb.py inside its region, as close as it can get to the parts it
connects to.

Board: 200 x 140 mm, 4 layers. The harness plugs enter the top edge.
"""
W, H = 200.0, 140.0

# TE 176122-6 header origin (pin row 1 at y = HY). The header mouth
# (harness side) is 25.5 mm above the pins and overhangs the top edge 0.5 mm.
HX, HY = 159.0, 25.0
TAIL_DY = 3.0          # tails row 1, 3 mm behind the header pins (rev C geometry)
TAIL_X0 = {"E8": -109.548, "E6": -72.901, "E5": -45.954}   # column 1 x, per plug

ANCHORS = {   # ref: (x, y, rotation deg)
    "J1": (HX, HY, 0),
    "J2": (HX + TAIL_X0["E5"], HY + TAIL_DY, 0),
    "J3": (HX + TAIL_X0["E6"], HY + TAIL_DY, 0),
    "J4": (HX + TAIL_X0["E8"], HY + TAIL_DY, 0),
    "U33": (20.0, 27.0, 0),        # ESP32-S3 module, antenna up at the top edge
    "J7": (171.0, 6.0, 0),         # AUX Micro-Fit 2x6, top edge right of the header
    "J5": (3.5, 62.0, 270),        # USB-C on the left edge
    "J6": (11.0, 88.0, 90),        # microSD, left edge
    "U100": (100.0, 104.0, 0),     # STM32H743 LQFP-144
}

# Regions for the floating parts, by sheet (x0, y0, x1, y1)
REGION = {
    "takeover": (40, 34, 199, 66),
    "injection": (150, 62, 199, 112),
    "outputs": (38, 60, 152, 84),
    "digital_in": (2, 46, 72, 139),
    "analog": (56, 84, 88, 139),
    "mcu": (76, 82, 132, 132),
    "safety": (118, 84, 152, 139),
    "comms": (2, 2, 40, 100),
    "power": (60, 112, 199, 139),
    "connectors": (40, 30, 160, 40),
}

# Large parts that get a fixed spot inside their region (x, y, rot)
FIXED = {
    # relays: FUEL (SPV, SVR), TIME (TCV), TRANS x2, AIR x2
    "K1": (183.0, 38.5, 0), "K2": (61.0, 38.5, 0), "K3": (164.0, 38.5, 0),
    "K4": (112.0, 37.0, 0), "K5": (126.0, 37.0, 0), "K6": (140.0, 37.0, 0), "K7": (88.0, 37.0, 0),
    # low-side drivers in a row under the relays
    "U18": (52.0, 74.0, 0), "U19": (70.0, 74.0, 0), "U20": (88.0, 74.0, 0), "U21": (106.0, 74.0, 0),
    "U22": (124.0, 74.0, 0),
    # muxes on the left, op-amps left of the MCU, VR chips right of it
    "U15": (30.0, 112.0, 0), "U16": (48.0, 112.0, 0), "U17": (30.0, 131.0, 90),
    "U8": (68.0, 94.0, 0), "U9": (68.0, 108.0, 0), "U10": (68.0, 122.0, 0),
    "U11": (126.0, 92.0, 0), "U12": (138.0, 92.0, 0),
    # spill / timing valve power stage
    "Q13": (186.0, 74.0, 0), "Q16": (186.0, 96.0, 0), "Q14": (166.0, 74.0, 0),
}

# M3 mounting holes. Top-left sits below the ESP32 antenna keep-out.
HOLES = ((4, 20), (W - 4, H - 4), (4, H - 4), (W - 4, 30))
