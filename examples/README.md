# ECU Data Access Examples

This folder contains example sketches for reading OEM ECU sensor data from your 1KZ-TE engine.

## 📁 Files

### `ecu_analog_intercept.ino` ⭐ RECOMMENDED
Reads sensor signals directly from the ECU by tapping the analog sensor wires (coolant temp, intake air temp, MAP/boost). Uses high-impedance voltage dividers to safely "listen in" without affecting ECU operation.

**Works on:** All 1KZ-TE vehicles (any year, any market)  
**Hardware:** ESP32 + 3× voltage dividers (100kΩ resistors)  
**Difficulty:** Medium (requires finding ECU connector pins)  
**Speed:** Fast (100Hz refresh rate)

### `ecu_obd2_bluetooth.ino`
Reads engine parameters via OBD-II protocol using an ELM327 Bluetooth adapter.

**Works on:** 1996+ USA/Canada 1KZ-TE with OBD-II compliance  
**Hardware:** ESP32 + ELM327 Bluetooth adapter  
**Difficulty:** Easy (plug and play)  
**Speed:** Slow (5-10Hz refresh rate)

**Note:** Most 1KZ-TE vehicles (especially pre-1996 or non-US) do NOT support OBD-II. Try the analog intercept method instead.

## 🚀 Quick Start

### Method 1: Analog Signal Intercept (Recommended)

1. **Build voltage dividers** for each sensor signal:
   ```
   ECU Pin → [100kΩ] → ESP32 GPIO → [100kΩ] → GND
   ```

2. **Find your ECU connector** (behind glove box typically)

3. **Identify sensor pins** with multimeter:
   - **THW (Coolant):** 0.5-4.5V, decreases as engine warms
   - **THA (Intake Air):** 0.5-4.5V, decreases as air heats up
   - **PIM (MAP):** 0.5-4.5V, changes with boost/vacuum

4. **Tap signals** with voltage dividers

5. **Upload sketch** `ecu_analog_intercept.ino`

6. **Calibrate** if needed (Toyota specs are pre-configured)

### Method 2: OBD-II Bluetooth

1. **Check compatibility:** Does your vehicle have an OBD-II port? (1996+ USA/Canada typically)

2. **Get ELM327 adapter:** Buy a quality Bluetooth adapter (avoid $5 clones)

3. **Pair adapter:** Turn ignition ON, plug in ELM327, wait for Bluetooth pairing

4. **Update sketch:** Change `ELM327_NAME` to match your adapter (line 45)

5. **Upload sketch** `ecu_obd2_bluetooth.ino`

6. **Test:** Open Serial Monitor, should see ECU data

## 📖 Full Documentation

For complete details on ECU data access methods, see:
- **[OEM_ECU_DATA_ACCESS.md](../docs/OEM_ECU_DATA_ACCESS.md)** - Comprehensive guide covering all methods

## ⚠️ Important Notes

### EGT Sensor Still Required
The 1KZ-TE ECU does **NOT** have an exhaust gas temperature (EGT) input. You **MUST** install a separate K-type thermocouple + MCP9600 module for EGT monitoring, regardless of which method you choose for other sensors.

### Safety Warnings
- ✅ Use high-impedance connections (>100kΩ) to avoid affecting ECU
- ✅ Verify signals with multimeter before connecting ESP32
- ✅ Do NOT cut or splice OEM wiring - use backprobing
- ✅ Disconnect battery when working on ECU connector
- ✅ Use proper tools (backprobing kit or scotchlok taps)

### Pre/Post Intercooler Temps
The 1KZ-TE ECU typically only has **one** intake air temperature sensor (usually pre-intercooler or in the manifold). If you want to monitor both pre and post intercooler temps separately, you'll need to install a second sensor using your I2C module.

## 🔧 Troubleshooting

### "ERROR" readings in analog intercept
- Check voltage divider resistor values (should be 100kΩ each)
- Verify you're reading the correct ECU pins (use multimeter)
- Ensure voltage multiplier is 2.0× in code (line 133)

### OBD-II won't connect
- Your 1KZ-TE may not support OBD-II (common for pre-1996 or non-US)
- Try a different ELM327 adapter (many clones are unreliable)
- Use the analog intercept method instead (works on all vehicles)

### Values don't match gauge cluster
- Adjust calibration constants in code (lines 29-37)
- Verify sensor specs for your specific year/model
- Use known temperatures (ice water, boiling water) for calibration

## 🆘 Need Help?

- Read the full guide: [OEM_ECU_DATA_ACCESS.md](../docs/OEM_ECU_DATA_ACCESS.md)
- Check the main project: [README.md](../README.md)
- Open an issue on GitHub

---

**Built with ❤️ for the turbodiesel community**
