"""Junkr ECU rev A: the whole circuit, as code.

This file is the single source of truth. gen_sch.py turns it into the KiCad
schematic and gen_pcb.py into the board. Every part has a sheet, a footprint,
a value and (where it matters) a manufacturer part number.

Net naming
  <PIN>      a harness signal shared by the harness, the stock-ECU tails and us
             (all inputs, wired-OR lamp/tach outputs, power, grounds)
  H_<PIN>    harness side of a takeover output (relay COM)
  S_<PIN>    stock-ECU side of a takeover output (relay NC, to the tails)
  O_<PIN>    our driver side of a takeover output (relay NO)
"""
import collections

from customsyms import NPINS, PAD0, pinout

# ---------------------------------------------------------------------------
# Footprints and part data
# ---------------------------------------------------------------------------
FP = dict(
    R0603="Resistor_SMD:R_0603_1608Metric", C0603="Capacitor_SMD:C_0603_1608Metric",
    C0805="Capacitor_SMD:C_0805_2012Metric", C1206="Capacitor_SMD:C_1206_3216Metric",
    C1210="Capacitor_SMD:C_1210_3225Metric", R2512="Resistor_SMD:R_2512_6332Metric",
    R1206="Resistor_SMD:R_1206_3216Metric",
    RA4="Resistor_SMD:R_Array_Convex_4x0603", SOT23="Package_TO_SOT_SMD:SOT-23",
    SOT235="Package_TO_SOT_SMD:SOT-23-5", SOT236="Package_TO_SOT_SMD:SOT-23-6",
    DPAK="Package_TO_SOT_SMD:TO-252-2", SMA="Diode_SMD:D_SMA", SMB="Diode_SMD:D_SMB",
    SMC="Diode_SMD:D_SMC", SOD123="Diode_SMD:D_SOD-123", LED="LED_SMD:LED_0603_1608Metric",
    SOIC8="Package_SO:SOIC-8_3.9x4.9mm_P1.27mm", SOIC14="Package_SO:SOIC-14_3.9x8.7mm_P1.27mm",
    SOIC24W="Package_SO:SOIC-24W_7.5x15.4mm_P1.27mm",
    XTAL="Crystal:Crystal_SMD_3225-4Pin_3.2x2.5mm", IND="Inductor_SMD:L_Sunlord_SWPA6045S",
    FB="Inductor_SMD:L_0603_1608Metric", PTC="Fuse:Fuse_1812_4532Metric",
    G5LE="Relay_THT:Relay_SPDT_Omron-G5LE-1", G5V2="Relay_THT:Relay_DPDT_Omron_G5V-2",
)

MPN = {  # value -> (manufacturer part number, note)
    "STM32H743ZIT6": ("STM32H743ZIT6", ""),
    "ESP32-S3-WROOM-1-N16R8": ("ESP32-S3-WROOM-1-N16R8", ""),
    "ATtiny1616-SN": ("ATTINY1616-SNR", ""),
    "MAX9926UAEE+": ("MAX9926UAEE+", ""),
    "DRV8803DW": ("DRV8803DWR", ""),
    "TPS7B4253QDDARQ1": ("TPS7B4253QDDARQ1", ""),
    "UCC27517ADBVR": ("UCC27517ADBVR", ""),
    "TPS54360B": ("TPS54360BDDAR", ""),
    "AP2112K-3.3": ("AP2112K-3.3TRG1", ""),
    "REF3030": ("REF3030AIDBZR", ""),
    "LM4040-5.0": ("LM4040DIM3-5.0/NOPB", "LM4040DBZ-5 footprint"),
    "MCP6004": ("MCP6004T-I/SL", ""),
    "LM2903": ("LM2903DR", ""),
    "INA240A1": ("INA240A1DR", ""),
    "CD74HC4067M": ("CD74HC4067M96", ""),
    "74LVC1G08": ("SN74LVC1G08DBVR", ""),
    "SN65HVD230": ("SN65HVD230DR", ""),
    "USBLC6-2SC6": ("USBLC6-2SC6", ""),
    "IPD50N10S3L-16": ("IPD50N10S3L16ATMA1", "100 V logic-level N-FET, DPAK"),
    "SUD50P10-43L": ("SUD50P10-43L-E3", "100 V P-FET, DPAK [VERIFY stock]"),
    "2N7002": ("2N7002", ""), "BSS123": ("BSS123", "100 V"),
    "AO3422": ("AO3422", "55 V 2 A"), "Si2309CDS": ("SI2309CDS-T1-GE3", "60 V P-FET"),
    "SMCJ58A": ("SMCJ58A", "SPV clamp"), "SMCJ33A": ("SMCJ33A", "load dump"),
    "SS310": ("SS310", "100 V 3 A Schottky"), "B360A": ("B360A-13-F", "buck catch diode"),
    "BAT54S": ("BAT54S", ""), "1N4148W": ("1N4148W", ""), "BZT52C12": ("BZT52C12", "12 V zener"),
    "G5LE-1-DC12": ("G5LE-1 DC12", "Omron SPDT 10 A"), "G5V-2-H1-DC12": ("G5V-2-H1 DC12", "Omron DPDT 2 A, 150 mW coil"),
    "25MHz": ("ABM8-25.000MHZ-B2-T", "3225, 18 pF"),
    "10uH": ("SWPA6045S100MT", "Sunlord 10 uH 3 A"),
    "PTC 1.1A": ("1812L110/33MR", "Littelfuse 33 V"),
    "TE 176122-6": ("176122-6", "or Ballenger CONN-100747"),
    "USB-C": ("TYPE-C-31-M-12", "HRO 16P"),
    "microSD": ("DM3AT-SF-PEJM5", "Hirose"),
    "Micro-Fit 2x6": ("43045-1212", "Molex vertical"),
    "SWD 2x5 1.27": ("FTSH-105-01-F-DV-K", "Samtec"),
}

Part = collections.namedtuple("Part", "ref lib value fp sheet fields")


class Circuit:
    def __init__(self):
        self.parts = []          # Part
        self.conns = {}          # ref -> {pin: net}
        self.counts = collections.Counter()
        self.nets = collections.defaultdict(list)
        self.dnp = set()
        self.nc = collections.defaultdict(set)   # ref -> pins deliberately unconnected
        self.sheet = None

    def part(self, prefix, lib, value, fp, pins=None, ref=None, **fields):
        if ref is None:
            self.counts[prefix] += 1
            ref = "%s%d" % (prefix, self.counts[prefix])
        f = dict(fields)
        if value in MPN and "MPN" not in f:
            f["MPN"] = MPN[value][0]
        p = Part(ref, lib, value, FP.get(fp, fp), self.sheet, f)
        self.parts.append(p)
        self.conns[ref] = {}
        if pins:
            self.connect(ref, pins)
        return ref

    def connect(self, ref, pins):
        for pin, net in pins.items():
            pin = str(pin)
            if net is None:
                self.nc[ref].add(pin)
                continue
            self.conns[ref][pin] = net
            self.nets[net].append((ref, pin))

    # -- shorthands -------------------------------------------------------
    def R(self, value, a, b, fp="R0603", **kw):
        return self.part("R", "Device:R", value, fp, {1: a, 2: b}, **kw)

    def C(self, value, a, b="GND", fp="C0603", **kw):
        return self.part("C", "Device:C", value, fp, {1: a, 2: b}, **kw)

    def RA(self, value, pairs, **kw):
        """4x0603 array. pairs = [(a, b)] * 4 (R1 1-8, R2 2-7, R3 3-6, R4 4-5)."""
        pins = {}
        for (a, b), (p, q) in zip(pairs, ((1, 8), (2, 7), (3, 6), (4, 5))):
            pins[p], pins[q] = a, b
        return self.part("RN", "Device:R_Pack04", value, "RA4", pins, **kw)

    def D(self, value, k, a, fp="SOD123", lib="Device:D", **kw):
        return self.part("D", lib, value, fp, {1: k, 2: a}, **kw)

    def LED(self, k, a, color, **kw):
        return self.part("D", "Device:LED", color, "LED", {1: k, 2: a}, **kw)

    def NFET(self, value, g, s, d, fp="SOT23", **kw):
        if fp == "DPAK":
            return self.part("Q", "Device:Q_NMOS_GDS", value, fp, {1: g, 2: d, 3: s}, **kw)
        return self.part("Q", "Device:Q_NMOS_GSD", value, fp, {1: g, 2: s, 3: d}, **kw)

    def PFET(self, value, g, s, d, fp="SOT23", **kw):
        if fp == "DPAK":
            return self.part("Q", "Device:Q_PMOS_GDS", value, fp, {1: g, 2: d, 3: s}, **kw)
        return self.part("Q", "Device:Q_PMOS_GSD", value, fp, {1: g, 2: s, 3: d}, **kw)

    def AND(self, a, b, y):
        u = self.part("U", "74xGxx:74LVC1G08", "74LVC1G08", "SOT235",
                      {1: a, 2: b, 4: y, 3: "GND", 5: "3V3"})
        self.C("100n", "3V3")
        return u

    def decouple(self, net, *values):
        for v in values:
            self.C(v, net, fp="C0805" if v in ("10u", "4u7", "22u") else "C0603")


c = Circuit()
PO = pinout()


def nm(plug, pin):
    n = PO[(plug, pin)]["name"]
    if not n:
        return "%s_%d_NC" % (plug, pin)
    n = {"L": "SHIFT_L", "2": "SHIFT_2", "P": "PATTERN"}.get(n, n)
    if n[-1] in "+-":                      # NE+ -> NEP, TDC- -> TDCN
        n = n[:-1] + ("P" if n[-1] == "+" else "N")
    return n.replace("/", "").replace("-", "").replace("+", "P")   # S-REL -> SREL, +B -> PB


# Takeover outputs: relay group -> Toyota pins
TAKEOVER = {
    "FUEL": [("E5", 11), ("E8", 13)],                 # SPV, SVR
    "TIME": [("E5", 12)],                             # TCV
    "TRANS": [("E5", 2), ("E5", 15), ("E5", 1)],      # S1, S2, SL
    "AIR": [("E5", 24), ("E5", 10), ("E5", 9), ("E8", 2)],   # EGR, S/TH1, S/TH2, S-REL
}
TAKE = {p: g for g, lst in TAKEOVER.items() for p in lst}


def harness_net(plug, pin):
    """Net on the header pin."""
    n = nm(plug, pin)
    if (plug, pin) in TAKE:
        return "H_" + n
    if n in ("E01", "E02", "E1"):
        return "GND"
    return n


def tail_net(plug, pin):
    n = nm(plug, pin)
    if (plug, pin) in TAKE:
        return "S_" + n
    if n in ("E01", "E02", "E1"):
        return "GND"
    return n


# ===========================================================================
# Sheet 1: ECU header, tails to the stock ECU, takeover relays
# ===========================================================================
c.sheet = "connectors"
hdr = {}
for plug in ("E8", "E6", "E5"):
    for pin in range(1, NPINS[plug] + 1):
        hdr[PAD0[plug] + pin] = harness_net(plug, pin)
c.part("J", "junkr_ecu:TE_176122-6", "TE 176122-6", "junkr_ecu:TE_176122-6_RA", hdr, ref="J1")
for k, plug in enumerate(("E5", "E6", "E8")):
    c.part("J", "junkr_ecu:TAILS_" + plug, "Tails %s" % plug, "junkr_ecu:Tails_" + plug,
           {pin: tail_net(plug, pin) for pin in range(1, NPINS[plug] + 1)}, ref="J%d" % (2 + k),
           note="shadow kit only: 150 mm tails to a new TE plug for the stock ECU")
# E2 (sensor ground) joins our ground through a 0 ohm link near the analog front end
c.R("0", "E2", "GND", fp="R0603")

# Takeover relays. NC = stock ECU, NO = ours, COM = harness. Coils on VRLY
# (switched +B, only live when ARM is high) and pulled low per group.
c.sheet = "takeover"
relays = []
for grp, lst in TAKEOVER.items():
    coil = "K_%s_N" % grp
    if grp in ("FUEL", "TIME"):
        for plug, pin in lst:
            n = nm(plug, pin)
            c.part("K", "Relay:G5LE-1", "G5LE-1-DC12", "G5LE",
                   {2: "VRLY", 5: coil, 1: "H_" + n, 4: "S_" + n, 3: "O_" + n})
    else:
        pairs = [lst[i:i + 2] for i in range(0, len(lst), 2)]
        for pr in pairs:
            pins = {1: "VRLY", 16: coil}
            (p1, q1) = pr[0]
            n1 = nm(p1, q1)
            pins.update({4: "H_" + n1, 6: "S_" + n1, 8: "O_" + n1})
            if len(pr) > 1:
                n2 = nm(*pr[1])
                pins.update({13: "H_" + n2, 11: "S_" + n2, 9: "O_" + n2})
            else:
                pins.update({13: None, 11: None, 9: None})
            c.part("K", "Relay:G5V-2", "G5V-2-H1-DC12", "G5V2", pins)
    c.D("1N4148W", "VRLY", coil)                             # coil flyback
    c.NFET("2N7002", "K_%s_G" % grp, "GND", coil)
    c.R("1k", "K_%s" % grp, "K_%s_G" % grp)                  # from the MCU
    c.R("100k", "K_%s_G" % grp, "GND")                       # off at reset
# VRLY = +B switched by ARM: P-FET high side, gate pulled down through an N-FET
c.PFET("Si2309CDS", "VRLY_G", "VIN", "VRLY")
c.R("47k", "VIN", "VRLY_G")
c.D("BZT52C12", "VIN", "VRLY_G")                              # Vgs clamp
c.NFET("2N7002", "ARM", "GND", "VRLY_GN")
c.R("4k7", "VRLY_G", "VRLY_GN")
c.C("1u", "VRLY", fp="C0805")

# ===========================================================================
# Sheet 2: power
# ===========================================================================
c.sheet = "power"
# +B (via ECD relay) and IGSW are diode-ORed into VIN; IGSW wakes us, then
# the MCU latches M-REL so the ECD relay feeds +B (stock behaviour).
c.D("SS310", "VIN_RAW", "PB", fp="SMA")
c.D("SS310", "VIN_RAW", "IGSW", fp="SMA")
c.part("F", "Device:Polyfuse", "PTC 1.1A", "PTC", {1: "VIN_RAW", 2: "VIN"})
c.D("SMCJ33A", "VIN", "GND", fp="SMC", lib="Device:D_TVS")
c.decouple("VIN", "100n")
c.C("2u2 100V", "VIN", fp="C1210")
c.C("2u2 100V", "VIN", fp="C1210")
# 5 V buck, TPS54360B: 500 kHz, Vout = 0.8 * (1 + 52.3k / 10k) = 4.98 V
c.part("U", "Regulator_Switching:TPS54360DDA", "TPS54360B", "Package_SO:TI_SO-PowerPAD-8_ThermalVias",
       {1: "BUCK_BOOT", 2: "VIN", 3: "BUCK_EN", 4: "BUCK_RT", 5: "BUCK_FB", 6: "BUCK_COMP",
        7: "GND", 8: "BUCK_SW", 9: "GND"})
c.R("100k", "VIN", "BUCK_EN")             # EN UVLO: start ~6.4 V
c.R("16k5", "BUCK_EN", "GND")
c.R("196k", "BUCK_RT", "GND")
c.C("100n", "BUCK_BOOT", "BUCK_SW")
c.D("B360A", "BUCK_SW", "GND", fp="SMA", lib="Device:D_Schottky")
c.part("L", "Device:L", "10uH", "IND", {1: "BUCK_SW", 2: "V5"})
c.R("52k3", "V5", "BUCK_FB")
c.R("10k", "BUCK_FB", "GND")
c.R("16k9", "BUCK_COMP", "BUCK_CC")
c.C("4n7", "BUCK_CC")
c.C("47p", "BUCK_COMP")
c.C("22u", "V5", fp="C1206")
c.C("22u", "V5", fp="C1206")
# Logic rails are fed from V5 or, on the bench, from USB VBUS (diode-OR)
c.D("SS310", "V5_OR", "V5", fp="SMA", lib="Device:D_Schottky")
c.D("SS310", "V5_OR", "VBUS", fp="SMA", lib="Device:D_Schottky")
c.C("10u", "V5_OR", fp="C0805")
for rail in ("3V3", "3V3_RF"):
    c.part("U", "Regulator_Linear:AP2112K-3.3", "AP2112K-3.3", "SOT235",
           {1: "V5_OR", 2: "GND", 3: "V5_OR", 4: None, 5: rail})
    c.C("1u", "V5_OR", fp="C0603")
    c.C("10u", rail, fp="C0805")
# ADC reference 3.000 V
c.part("U", "Reference_Voltage:REF3030", "REF3030", "SOT23", {1: "V5", 2: "VREF", 3: "GND"})
c.C("1u", "V5")
c.C("1u", "VREF")
c.C("100n", "VREF")
# Sensor 5 V (VC): tracking LDO, ADJ = 5.000 V from an LM4040, off unless the
# MCU drives VC_EN low... we invert: VC_OFF pulls ADJ to ground.
c.part("U", "junkr_ecu:TPS7B4253DDA", "TPS7B4253QDDARQ1", "Package_SO:HSOP-8-1EP_3.9x4.9mm_P1.27mm_EP2.41x3.1mm",
       {8: "VIN", 5: "VC_ADJ", 1: "VC", 4: "VC", 3: "GND", 6: "GND", 9: "GND", 2: None, 7: None})
c.R("4k7", "VIN", "REF5")
c.part("U", "Reference_Voltage:LM4040DBZ-5", "LM4040-5.0", "SOT23", {1: "REF5", 2: "GND"})
c.C("1u", "REF5")
c.R("10k", "REF5", "VC_ADJ")
c.C("100n", "VC_ADJ")
c.NFET("2N7002", "VC_OFF", "GND", "VC_ADJ")
c.R("100k", "3V3", "VC_OFF")              # VC off at reset (shadow-safe)
c.R("1k", "VC_OFF_MCU", "VC_OFF")
c.C("10u", "VC", fp="C0805")
c.C("100n", "VC")
# Power LED
c.LED("PWR_LED", "3V3", "green")
c.R("1k", "PWR_LED", "GND")

# ===========================================================================
# Sheet 3: STM32H743 (engine CPU)
# ===========================================================================
c.sheet = "mcu"
MCU_PINS = {  # pin name -> net. Checked against the symbol's alternate functions.
    # Crank and pump timing: everything on 32-bit TIM2 so edges share one clock
    "PA15": "NE_DIG",        # TIM2_CH1  pump speed (NE) teeth
    "PB3": "TDC_DIG",        # TIM2_CH2  crank TDC
    "PB10": "SPV_CMP",       # TIM2_CH3  harness SPV edges (stock or ours)
    "PB11": "TCV_CMP",       # TIM2_CH4  harness TCV edges
    "PD12": "SP2_DIG",       # TIM4_CH1  A/T output speed
    "PD13": "SP1_CMP",       # TIM4_CH2  vehicle speed
    "PD14": "TAC_CMP",       # TIM4_CH3  tach (stock in shadow mode)
    "PD15": "VRAUX_DIG",     # TIM4_CH4  spare VR input
    # Actuators
    "PE9": "SPV_CMD",        # TIM1_CH1  spill valve
    "PE11": "SPV_REC",       # TIM1_CH2  spill valve recirculation (hold)
    "PC6": "TCV_CMD",        # TIM8_CH1  timing control valve PWM
    "PE5": "DRV_TAC",        # TIM15_CH1 tach output
    # Analog
    "PA0": "PIM_A", "PA3": "VA_A", "PA4": "THW_A", "PA5": "THA_A", "PA6": "THF_A",
    "PA7": "THO_A", "PB1": "VRP_A", "PC0": "VRT_A", "PC1": "VC_A", "PC4": "E2_A",
    "PC5": "AUX1_A", "PF11": "AUX2_A", "PB0": "ISPV", "PF12": "ITCV", "PF13": "VB_A",
    "PF14": "V5_A", "PF3": "SPV_V", "PF4": "TCV_V", "PF5": "MUXA", "PF6": "MUXB",
    "PF7": "MUXC", "PF8": "TBOARD",
    # Low-side drivers (DRV8803 inputs)
    "PG0": "DRV_S1", "PG1": "DRV_W", "PG2": "DRV_OILW", "PG3": "DRV_GIND",
    "PG4": "DRV_S2", "PG5": "DRV_SNWL", "PG6": "DRV_VF", "PG7": "DRV_TT",
    "PG8": "DRV_SL", "PG9": "DRV_IMO", "PG10": "DRV_ACT",
    "PG11": "DRV_EGR", "PG12": "DRV_STH1", "PG13": "DRV_STH2", "PG14": "DRV_AC",
    "PG15": "DRV_SREL", "PE0": "DRV_LS1", "PE1": "DRV_LS2", "PE2": "DRV_LS3",
    "PE4": "DRV_ENBL_N", "PE6": "DRV_RESET", "PE7": "DRV_FAULT_N",
    # Relays, power, safety
    "PD3": "K_FUEL", "PD4": "K_TIME", "PD5": "K_TRANS", "PD6": "K_AIR",
    "PD7": "SVR_CMD", "PD8": "MREL_CMD", "PD9": "VC_OFF_MCU", "PD10": "PU_EN",
    "PD11": "ARM_REQ", "PE8": "HB", "PE10": "ARM", "PE12": "SAFE_OK",
    "PF0": "MUX_S0", "PF1": "MUX_S1", "PF2": "MUX_S2", "PF15": "MUX_S3",
    "PB4": "LED_RUN", "PB5": "LED_FAULT",
    # Comms
    "PA9": "STM_TX", "PA10": "STM_RX",                  # USART1 (also the ROM bootloader)
    "PB12": "SPI_CS", "PB13": "SPI_SCK", "PB14": "SPI_MISO", "PB15": "SPI_MOSI",  # SPI2 slave
    "PB6": "STM_IRQ",
    "PD0": "CAN_RX", "PD1": "CAN_TX",                    # FDCAN1
    "PA13": "SWDIO", "PA14": "SWCLK",
    "PH0": "HSE_IN", "PH1": "HSE_OUT",
}
MCU_AF = {  # pin -> alternate function that must exist on it
    "PA15": "TIM2_CH1", "PB3": "TIM2_CH2", "PB10": "TIM2_CH3", "PB11": "TIM2_CH4",
    "PD12": "TIM4_CH1", "PD13": "TIM4_CH2", "PD14": "TIM4_CH3", "PD15": "TIM4_CH4",
    "PE9": "TIM1_CH1", "PE11": "TIM1_CH2", "PC6": "TIM8_CH1", "PE5": "TIM15_CH1",
    "PB0": "COMP1_INP", "PA9": "USART1_TX", "PA10": "USART1_RX", "PB12": "SPI2_NSS",
    "PB13": "SPI2_SCK", "PB14": "SPI2_MISO", "PB15": "SPI2_MOSI", "PD0": "FDCAN1_RX",
    "PD1": "FDCAN1_TX", "PA13": "DEBUG_JTMS-SWDIO", "PA14": "DEBUG_JTCK-SWCLK",
    "PH0": "RCC_OSC_IN", "PH1": "RCC_OSC_OUT",
}
ADC_PINS = ["PA0", "PA3", "PA4", "PA5", "PA6", "PA7", "PB1", "PC0", "PC1", "PC4", "PC5", "PF11",
            "PB0", "PF12", "PF13", "PF14", "PF3", "PF4", "PF5", "PF6", "PF7", "PF8"]
MCU_POWER = {"VDD": "3V3", "VSS": "GND", "VDDA": "3V3A", "VSSA": "GND", "VREF+": "VREF",
             "VBAT": "3V3", "VCAP": "VCAP", "PDR_ON": "3V3", "VDD33_USB": "3V3",
             "BOOT0": "BOOT0", "NRST": "NRST"}
MCU_REF = "U100"
c.part("U", "MCU_ST_STM32H7:STM32H743ZITx", "STM32H743ZIT6", "Package_QFP:LQFP-144_20x20mm_P0.5mm",
       ref=MCU_REF)   # pins are connected in finish_mcu() once the symbol is known
for _ in range(10):
    c.C("100n", "3V3")
c.C("4u7", "3V3", fp="C0805")
c.C("2u2", "VCAP")
c.C("2u2", "VCAP")
c.part("FB", "Device:FerriteBead_Small", "600R@100MHz", "FB", {1: "3V3", 2: "3V3A"})
c.C("1u", "3V3A")
c.C("100n", "3V3A")
c.part("Y", "Device:Crystal_GND24", "25MHz", "XTAL", {1: "HSE_IN", 3: "HSE_OUT", 2: "GND", 4: "GND"})
c.C("12p", "HSE_IN")
c.C("12p", "HSE_OUT")
c.R("10k", "BOOT0", "GND")
c.C("100n", "NRST")
c.R("10k", "3V3", "NRST")
c.part("J", "Connector_Generic:Conn_02x05_Odd_Even", "SWD 2x5 1.27",
       "Connector_PinHeader_1.27mm:PinHeader_2x05_P1.27mm_Vertical_SMD",
       {1: "3V3", 2: "SWDIO", 3: "GND", 4: "SWCLK", 5: "GND", 6: None, 7: None, 8: None, 9: "GND", 10: "NRST"},
       ref="J10")
c.LED("LED_RUN_K", "3V3", "green")
c.R("1k", "LED_RUN", "LED_RUN_K")
c.LED("LED_FAULT_K", "3V3", "red")
c.R("1k", "LED_FAULT", "LED_FAULT_K")
c.R("10k", "3V3", "DRV_FAULT_N")
# Board temperature NTC (10k B3435) against 10k to VREF
c.R("10k", "VREF", "TBOARD")
c.part("TH", "Device:Thermistor_NTC", "10k NTC", "R0603", {1: "TBOARD", 2: "GND"},
       MPN="NCP18XH103F03RB")
c.C("100n", "TBOARD")
# CAN
c.part("U", "Interface_CAN_LIN:SN65HVD230", "SN65HVD230", "SOIC8",
       {1: "CAN_TX", 2: "GND", 3: "3V3", 4: "CAN_RX", 5: None, 6: "CANL", 7: "CANH", 8: "CAN_RS"})
c.R("0", "CAN_RS", "GND")
c.C("100n", "3V3")
c.R("120", "CANH", "CAN_TERM", note="DNP unless this is the end of the bus")
c.R("0", "CAN_TERM", "CANL", note="solder link: closes the 120 ohm terminator")

# ===========================================================================
# Sheet 4: analog inputs (high impedance, so the stock ECU never notices us)
# ===========================================================================
c.sheet = "analog"
# 12 buffered channels: harness pin -> 47k -> MCP6004 follower (5 V) -> 10k/15k -> ADC
ANALOG = [("PIM", "PIM_A"), ("VA", "VA_A"), ("THW", "THW_A"), ("THA", "THA_A"), ("THF", "THF_A"),
          ("THO", "THO_A"), ("VRP", "VRP_A"), ("VRT", "VRT_A"), ("VC", "VC_A"), ("E2", "E2_A"),
          ("AUX_AN1", "AUX1_A"), ("AUX_AN2", "AUX2_A")]
opamp_units = [(3, 2, 1), (5, 6, 7), (10, 9, 8), (12, 13, 14)]   # (+, -, out)
for chip in range(3):
    chans = ANALOG[chip * 4:chip * 4 + 4]
    pins = {4: "V5", 11: "GND"}
    for (sig, adc), (pp, pn, po) in zip(chans, opamp_units):
        pins[pp] = sig + "_B"
        pins[pn] = sig + "_O"
        pins[po] = sig + "_O"
    c.part("U", "Amplifier_Operational:MCP6004", "MCP6004", "SOIC14", pins)
    c.C("100n", "V5")
    c.RA("47k", [(s, s + "_B") for s, _ in chans])
    c.RA("10k", [(s + "_O", a) for s, a in chans])
    c.RA("15k", [(a, "GND") for _, a in chans])
    for _, a in chans:
        c.C("10n", a)
# Standalone pull-ups (stock ECU absent): 2.49k from VPU to each resistive sensor
for sig in ("THA", "THW", "THF", "THO", "VRP", "VRT"):
    c.R("2k49 1%", "VPU", sig)
c.PFET("Si2309CDS", "VPU_G", "VC", "VPU")
c.R("100k", "VC", "VPU_G")
c.NFET("2N7002", "PU_EN", "GND", "VPU_G")
c.R("100k", "PU_EN", "GND")
# Supplies and fast actuator voltages
c.R("100k", "PB", "VB_A")
c.R("15k", "VB_A", "GND")
c.C("10n", "VB_A")
c.R("10k", "V5", "V5_A")
c.R("10k", "V5_A", "GND")
c.C("10n", "V5_A")
for sig in ("H_SPV", "H_TCV"):     # 1M/22k: 150 V -> 3.2 V, high impedance
    a = "SPV_V" if "SPV" in sig else "TCV_V"
    c.R("1M", sig, a)
    c.R("22k", a, "GND")
    c.C("1n", a)
    c.part("D", "Diode:BAT54S", "BAT54S", "SOT23", {1: "GND", 2: "VREF", 3: a})

# ===========================================================================
# Sheet 5: digital inputs (VR sensors, comparators, input muxes)
# ===========================================================================
c.sheet = "digital_in"
# VR sensors: MAX9926 in Mode A2 (internal bias, adaptive threshold + zero crossing)
for k, (a, b) in enumerate((("NE", "TDC"), ("SP2", "VRAUX"))):
    pins = {14: "V5", 11: "GND", 3: "GND", 6: "GND", 1: "GND", 8: "GND", 13: "GND",
            2: None, 7: None, 12: None, 4: a + "_DIG", 5: b + "_DIG"}
    pins[16], pins[15] = a + "P_F", a + "N_F"
    pins[9], pins[10] = b + "P_F", b + "N_F"
    c.part("U", "junkr_ecu:MAX9926", "MAX9926UAEE+", "Package_SO:QSOP-16_3.9x4.9mm_P0.635mm", pins)
    c.C("10n", "V5")
    c.C("100n", "V5")
    for s in (a, b):
        src_p, src_n = (s + "P", s + "N") if s != "VRAUX" else ("VRAUX_P", "VRAUX_N")
        c.R("10k", src_p, s + "P_F")
        c.R("10k", src_n, s + "N_F")
        c.C("1n", s + "P_F", s + "N_F")
        c.R("10k", "3V3", s + "_DIG")
# Edge comparators: LM2903, inputs from 100k/22k dividers, threshold 0.43 V
CMP = [("H_SPV", "SPV_CMP"), ("H_TCV", "TCV_CMP"), ("SP1", "SP1_CMP"), ("TAC", "TAC_CMP")]
c.R("10k", "3V3", "CMP_REF")
c.R("1k5", "CMP_REF", "GND")
c.C("100n", "CMP_REF")
for chip in range(2):
    ch = CMP[chip * 2:chip * 2 + 2]
    pins = {8: "V5", 4: "GND"}
    for (src, out), (pp, pn, po) in zip(ch, ((3, 2, 1), (5, 6, 7))):
        node = out + "_IN"
        pins[pp], pins[pn], pins[po] = node, "CMP_REF", out
        c.R("100k", src, node)
        c.R("22k", node, "GND")
        c.R("1M", out, node)              # hysteresis
        c.R("4k7", "3V3", out)
    c.part("U", "Comparator:LM2903", "LM2903", "SOIC8", pins)
    c.C("100n", "V5")
# Slow inputs: three CD74HC4067 muxes, every channel 100k/22k (18 V -> 3.25 V)
MUX = {
    "MUXA": ["IDL", "STP", "AC", "STA", "IGSW", "PATTERN", "OD2", "IMI", "ACT", "HSW", "NSW",
             "L4", "TFN", "SHIFT_L", "SHIFT_2", "SNW"],
    "MUXB": ["TE1", "TE2", "TT", "H_S1", "H_S2", "H_SL", "H_EGR", "H_STH1", "H_STH2", "H_SREL",
             "W", "GIND", "OILW", "SNWL", "MREL", "H_SVR"],
    "MUXC": ["TAC", "VF", "IMO", "H_SPV", "H_TCV", "BATT", "VIN", "K_FUEL_N", "K_TIME_N",
             "K_TRANS_N", "K_AIR_N", "O_SPV", "O_TCV", "ARM", "RUN_SW", "VRLY"],
}
for mux, sigs in MUX.items():
    pins = {1: mux, 10: "MUX_S0", 11: "MUX_S1", 14: "MUX_S2", 13: "MUX_S3", 15: "GND",
            12: "GND", 24: "3V3"}
    ins = [9, 8, 7, 6, 5, 4, 3, 2, 23, 22, 21, 20, 19, 18, 17, 16]  # I0..I15
    nodes = []
    for i, s in enumerate(sigs):
        node = "%s_%d" % (mux, i)
        pins[ins[i]] = node
        nodes.append((s, node))
    c.part("U", "74xx:CD74HC4067M", "CD74HC4067M", "SOIC24W", pins)
    c.C("100n", "3V3")
    for g in range(4):
        grp = nodes[g * 4:g * 4 + 4]
        c.RA("100k", [(s, n) for s, n in grp])
        c.RA("22k", [(n, "GND") for _, n in grp])

# ===========================================================================
# Sheet 6: low-side outputs (DRV8803 x5)
# ===========================================================================
c.sheet = "outputs"
DRV = [  # (DRV input net, output net) x4 per chip; heavy loads one per chip
    [("DRV_S1", "O_S1"), ("DRV_W", "W"), ("DRV_OILW", "OILW"), ("DRV_GIND", "GIND")],
    [("DRV_S2", "O_S2"), ("DRV_SNWL", "SNWL"), ("DRV_VF", "VF"), ("DRV_TT", "TT")],
    [("DRV_SL", "O_SL"), ("DRV_TAC", "TAC"), ("DRV_IMO", "IMO"), ("DRV_ACT", "ACT")],
    [("DRV_EGR", "O_EGR"), ("DRV_STH1", "O_STH1"), ("DRV_STH2", "O_STH2"), ("DRV_AC", "AC")],
    [("DRV_SREL", "O_SREL"), ("DRV_LS1", "LS1"), ("DRV_LS2", "LS2"), ("DRV_LS3", "LS3")],
]
for chip in DRV:
    pins = {1: "PB", 2: "PB", 10: "DRV_ENBL_N", 11: "DRV_RESET", 20: "DRV_FAULT_N", 19: None}
    for g in (5, 6, 7, 14, 15, 16):
        pins[g] = "GND"
    for (inp, out), (pi, po) in zip(chip, ((18, 3), (17, 4), (13, 8), (12, 9))):
        pins[pi], pins[po] = inp, out
    c.part("U", "junkr_ecu:DRV8803DW", "DRV8803DW", "Package_SO:SOIC-20W_7.5x12.8mm_P1.27mm", pins)
    c.C("100n", "PB")
    c.C("1u 50V", "PB", fp="C0805")
c.R("10k", "3V3", "DRV_ENBL_N")     # disabled until the MCU enables
c.R("10k", "DRV_RESET", "GND")
# M-REL: high side from VIN, diode-ORed with the stock ECU's M-REL
c.PFET("Si2309CDS", "MREL_G", "VIN", "MREL_SRC")
c.R("47k", "VIN", "MREL_G")
c.D("BZT52C12", "VIN", "MREL_G")
c.NFET("2N7002", "MREL_CMD", "GND", "MREL_GN")
c.R("4k7", "MREL_G", "MREL_GN")
c.R("100k", "MREL_CMD", "GND")
c.D("SS310", "MREL", "MREL_SRC", fp="SMA", lib="Device:D_Schottky")
# SVR (spill valve relay coil): low side, only when ARM
c.NFET("AO3422", "SVR_G", "GND", "O_SVR")
c.R("100k", "SVR_G", "GND")
c.D("SS310", "PB", "O_SVR", fp="SMA", lib="Device:D_Schottky")

# ===========================================================================
# Sheet 7: injection drivers (spill valve, timing valve)
# ===========================================================================
c.sheet = "injection"
# SPV: peak and hold. Q1 low side (gate driver), Q2 + D recirculate during
# hold (slow decay); both off = fast decay into the 58 V TVS (sharp end of
# injection). Current: 20 mOhm shunt -> INA240A1 (x20) -> ISPV (COMP1 + ADC).
c.part("U", "junkr_ecu:UCC27517A", "UCC27517ADBVR", "SOT235",
       {1: "V5", 2: "GND", 3: "SPV_IN", 4: "GND", 5: "SPV_GATE_R"})
c.C("1u", "V5")
c.R("4R7", "SPV_GATE_R", "SPV_GATE")
c.R("10k", "SPV_GATE", "GND")
c.NFET("IPD50N10S3L-16", "SPV_GATE", "SPV_SHUNT", "O_SPV", fp="DPAK")
c.R("0R02 2W", "SPV_SHUNT", "GND", fp="R2512")
c.D("SMCJ58A", "O_SPV", "GND", fp="SMC", lib="Device:D_TVS")
c.D("SS310", "SPV_REC_N", "O_SPV", fp="SMA", lib="Device:D_Schottky")
c.PFET("SUD50P10-43L", "SPV_REC_G", "SPV_REC_N", "PB", fp="DPAK")
c.R("10k", "SPV_REC_N", "SPV_REC_G")
c.D("BZT52C12", "SPV_REC_N", "SPV_REC_G")
c.NFET("BSS123", "SPV_REC", "GND", "SPV_REC_GN")
c.R("2k2", "SPV_REC_G", "SPV_REC_GN")
c.R("100k", "SPV_REC", "GND")
c.part("U", "Amplifier_Current:INA240A1D", "INA240A1", "SOIC8",
       {8: "SPV_SHUNT", 1: "GND", 2: "GND", 3: "GND", 7: "GND", 4: "GND", 6: "V5", 5: "ISPV_O"})
c.C("100n", "V5")
c.R("10k", "ISPV_O", "ISPV")
c.R("15k", "ISPV", "GND")
c.C("1n", "ISPV")
c.AND("SPV_CMD", "ARM", "SPV_IN")
c.R("100k", "SPV_CMD", "GND")
# TCV: PWM low side, freewheel diode to +B, 50 mOhm shunt -> INA240A1
c.part("U", "junkr_ecu:UCC27517A", "UCC27517ADBVR", "SOT235",
       {1: "V5", 2: "GND", 3: "TCV_IN", 4: "GND", 5: "TCV_GATE_R"})
c.C("1u", "V5")
c.R("10R", "TCV_GATE_R", "TCV_GATE")
c.R("10k", "TCV_GATE", "GND")
c.NFET("IPD50N10S3L-16", "TCV_GATE", "TCV_SHUNT", "O_TCV", fp="DPAK")
c.R("0R05 1W", "TCV_SHUNT", "GND", fp="R2512")
c.D("SS310", "PB", "O_TCV", fp="SMA", lib="Device:D_Schottky")
c.part("U", "Amplifier_Current:INA240A1D", "INA240A1", "SOIC8",
       {8: "TCV_SHUNT", 1: "GND", 2: "GND", 3: "GND", 7: "GND", 4: "GND", 6: "V5", 5: "ITCV_O"})
c.C("100n", "V5")
c.R("10k", "ITCV_O", "ITCV")
c.R("15k", "ITCV", "GND")
c.C("1n", "ITCV")
c.AND("TCV_CMD", "ARM", "TCV_IN")
c.R("100k", "TCV_CMD", "GND")

# ===========================================================================
# Sheet 8: safety (ATtiny1616 supervisor, ARM chain, RUN switch)
# ===========================================================================
c.sheet = "safety"
# ARM = ARM_REQ (STM32) & SAFE_OK (ATtiny) & RUN_SW (dash switch). ARM powers
# the takeover relays and gates SPV, TCV and SVR. Any input low = stock ECU
# back in charge (shadow kit) or no fuel (standalone).
c.AND("ARM_REQ", "SAFE_OK", "ARM1")
c.AND("ARM1", "RUN_SW", "ARM")
c.AND("SVR_CMD", "ARM", "SVR_G")
c.R("100k", "ARM_REQ", "GND")
c.R("100k", "SAFE_OK", "GND")
c.R("100k", "SVR_CMD", "GND")
c.R("10k", "RUN_SW", "GND")                       # switch open = STOCK
c.part("J", "Connector_Generic:Conn_01x02", "RUN switch", "Connector_JST:JST_PH_B2B-PH-K_1x02_P2.00mm_Vertical",
       {1: "RUN_PWR", 2: "RUN_SW"}, ref="J11")
c.R("1k", "3V3", "RUN_PWR")
c.part("U", "MCU_Microchip_ATtiny:ATtiny1616-S", "ATtiny1616-SN", "Package_SO:SOIC-20W_7.5x12.8mm_P1.27mm",
       {1: "3V3", 20: "GND", 16: "UPDI", 17: "NE_DIG", 18: "SPV_IN", 19: "SPV_CMP", 2: "HB",
        3: "TDC_DIG", 4: "ARM", 5: "SAFE_OK_T", 6: "TINY_LED", 7: None, 8: "TINY_RX", 9: "TINY_TX",   # PB3 = USART0 RXD, PB2 = TXD
        10: None, 11: None, 12: None, 13: None, 14: None, 15: None})
c.C("100n", "3V3")
c.R("1k", "SAFE_OK_T", "SAFE_OK")
c.part("J", "Connector_Generic:Conn_01x03", "UPDI", "Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical",
       {1: "3V3", 2: "UPDI", 3: "GND"}, ref="J12")
c.R("1k", "UPDI", "ESP_UPDI")
c.LED("TINY_LED_K", "3V3", "yellow")
c.R("1k", "TINY_LED", "TINY_LED_K")
# ARMED indicator
c.NFET("2N7002", "ARM", "GND", "ARM_LED_K")
c.LED("ARM_LED_K", "ARM_LED_A", "red")
c.R("1k", "3V3", "ARM_LED_A")

# ===========================================================================
# Sheet 9: ESP32-S3 (Wi-Fi tuner, logger, STM32 programmer), USB, SD, AUX
# ===========================================================================
c.sheet = "comms"
c.part("U", "RF_Module:ESP32-S3-WROOM-1", "ESP32-S3-WROOM-1-N16R8", "RF_Module:ESP32-S3-WROOM-1",
       {1: "GND", 40: "GND", 41: "GND", 2: "3V3_RF", 3: "ESP_EN", 27: "ESP_IO0",
        13: "USB_DN", 14: "USB_DP",
        4: "SPI_SCK", 5: "SPI_MOSI", 6: "SPI_MISO", 7: "SPI_CS",          # IO4-7 to STM32 SPI2
        8: "STM_RX", 9: "STM_TX",                                          # IO15 -> STM RX, IO16 <- STM TX
        10: "NRST_ESP", 11: "BOOT0",                                       # IO17, IO18
        12: "STM_IRQ",                                                     # IO8
        22: "ESP_UPDI",                                                    # IO14
        18: "SD_CS", 19: "SD_MOSI", 20: "SD_SCK", 21: "SD_MISO",           # IO10-13
        38: "ESP_LED",                                                     # IO2
        15: None, 16: None, 17: None, 23: "TINY_TX", 24: "TINY_RX", 25: None, 26: None, 28: None, 29: None,
        30: None, 31: None, 32: None, 33: None, 34: None, 35: None, 36: None, 37: None, 39: None})
c.C("10u", "3V3_RF", fp="C0805")
c.C("100n", "3V3_RF")
c.R("10k", "3V3_RF", "ESP_EN")
c.C("1u", "ESP_EN")
c.R("10k", "3V3_RF", "ESP_IO0")
c.part("SW", "Switch:SW_Push", "BOOT", "Button_Switch_SMD:SW_SPST_PTS810", {1: "ESP_IO0", 2: "GND"})
# ESP pulls STM32 NRST through a transistor (open drain)
c.NFET("2N7002", "NRST_ESP", "GND", "NRST")
c.R("100k", "NRST_ESP", "GND")
c.LED("ESP_LED_K", "3V3_RF", "blue")
c.R("1k", "ESP_LED", "ESP_LED_K")
# USB-C (device, bench power + ESP32 native USB)
c.part("J", "Connector:USB_C_Receptacle_USB2.0_16P", "USB-C", "Connector_USB:USB_C_Receptacle_HRO_TYPE-C-31-M-12",
       {"A1": "GND", "A12": "GND", "B1": "GND", "B12": "GND", "A4": "VBUS", "A9": "VBUS",
        "B4": "VBUS", "B9": "VBUS", "A5": "CC1", "B5": "CC2", "A6": "USB_CP", "B6": "USB_CP",
        "A7": "USB_CN", "B7": "USB_CN", "A8": None, "B8": None, "S1": "GND"}, ref="J5")
c.R("5k1", "CC1", "GND")
c.R("5k1", "CC2", "GND")
c.part("U", "Power_Protection:USBLC6-2SC6", "USBLC6-2SC6", "SOT236",
       {1: "USB_CP", 6: "USB_DP", 3: "USB_CN", 4: "USB_DN", 2: "GND", 5: "VBUS"})
c.C("4u7", "VBUS", fp="C0805")
# microSD (logs)
c.part("J", "Connector:Micro_SD_Card_Det_Hirose_DM3AT", "microSD", "Connector_Card:microSD_HC_Hirose_DM3AT-SF-PEJM5",
       {1: "SD_D2", 2: "SD_CS", 3: "SD_MOSI", 4: "3V3_RF", 5: "SD_SCK", 6: "GND", 7: "SD_MISO", 8: "SD_D1",
        9: None, 10: None, 11: "GND"}, ref="J6")
c.RA("10k", [("3V3_RF", "SD_CS"), ("3V3_RF", "SD_MOSI"), ("3V3_RF", "SD_MISO"), ("3V3_RF", "SD_D1")])
c.R("10k", "3V3_RF", "SD_D2")
c.C("100n", "3V3_RF")
# AUX connector: CAN, switched 12 V, sensor 5 V, two analog ins, three low-side
# outputs, a spare VR input
c.part("F", "Device:Polyfuse", "PTC 0.5A", "PTC", {1: "VIN", 2: "AUX_12V"}, MPN="1812L050/30PR")
c.part("J", "Connector_Generic:Conn_02x06_Odd_Even", "Micro-Fit 2x6",
       "Connector_Molex:Molex_Micro-Fit_3.0_43045-1212_2x06_P3.00mm_Vertical",
       {1: "CANH", 2: "CANL", 3: "AUX_12V", 4: "GND", 5: "VC", 6: "GND", 7: "AUX_AN1", 8: "AUX_AN2",
        9: "LS1", 10: "LS2", 11: "LS3", 12: "VRAUX_P"}, ref="J7")
c.R("0", "VRAUX_N", "GND", note="spare VR input is single ended on the AUX plug")


# ---------------------------------------------------------------------------
# MCU pins (needs the symbol, so done after the lib is readable)
# ---------------------------------------------------------------------------
def finish_mcu():
    import symlib
    sym = symlib.load("MCU_ST_STM32H7:STM32H743ZITx")
    pins = symlib.pins(sym)
    alts = {}
    import sexpr
    for sub in sexpr.findall(sym, "symbol"):
        for p in sexpr.findall(sub, "pin"):
            nmx = str(sexpr.find(p, "name")[1])
            alts[nmx] = {str(a[1]) for a in sexpr.findall(p, "alternate")}
    errors = []
    for pin, af in MCU_AF.items():
        if af not in alts.get(pin, set()):
            errors.append("%s has no %s (has %s)" % (pin, af, sorted(a for a in alts.get(pin, ()) if af.split('_')[0] in a)))
    for pin in ADC_PINS:
        if not any(a.startswith("ADC") for a in alts.get(pin, ())):
            errors.append("%s is not an ADC pin" % pin)
    conn = {}
    for p in pins:
        if p.name in MCU_PINS:
            conn[p.number] = MCU_PINS[p.name]
        elif p.name in MCU_POWER:
            conn[p.number] = MCU_POWER[p.name]
        else:
            conn[p.number] = None
    used = {p.name for p in pins}
    for k in MCU_PINS:
        if k not in used:
            errors.append("%s not on the package" % k)
    c.connect(MCU_REF, conn)
    return errors


ERRORS = finish_mcu()


def check():
    """Electrical sanity: nets with one pin, rails, duplicates."""
    issues = list(ERRORS)
    ok_single = {"VRAUX_N"}
    for n, lst in c.nets.items():
        if len(lst) < 2 and n not in ok_single:
            issues.append("net %s has only %s" % (n, lst))
    return issues


if __name__ == "__main__":
    for i in check():
        print("CHECK:", i)
    print(len(c.parts), "parts,", len(c.nets), "nets")
