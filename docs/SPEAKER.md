# Speaker Module Setup Guide

## Overview

The JNKR Gauge System uses a **DFPlayer Mini MP3 module** to play preloaded audio alerts for various engine conditions. This provides clear, verbal warnings for critical thresholds and different alert levels.

---

## Hardware Requirements

### DFPlayer Mini Module
- **Module**: DFPlayer Mini MP3 Player
- **Communication**: Serial (UART)
- **Power**: 3.3V - 5V DC
- **Storage**: Micro SD card (up to 32GB)
- **Audio Output**: 3W stereo output
- **Purchase**: Available from most electronics suppliers (~$3-5 USD)

### Additional Components
- **Micro SD Card**: 8GB or smaller (formatted as FAT32)
- **Speaker**: 3W 4Ω or 8Ω speaker
- **Optional**: Small amplifier if more volume is needed
- **Wires**: Jumper wires for connections

---

## Wiring Diagram

### DFPlayer Mini to Arduino Mega 2560

```
DFPlayer Mini          Arduino Mega 2560
──────────────────────────────────────────
VCC (Pin 1)      →     5V
RX  (Pin 2)      →     Pin 8 (TX)
TX  (Pin 3)      →     Pin 9 (RX)
SPK_1 (Pin 6)    →     Speaker (+)
GND (Pin 7)      →     GND
SPK_2 (Pin 8)    →     Speaker (-)
GND (Pin 10)     →     GND
```

### Pin Configuration
- **PIN_SPEAKER_TX**: Digital Pin 8 (Arduino TX → DFPlayer RX)
- **PIN_SPEAKER_RX**: Digital Pin 9 (Arduino RX → DFPlayer TX)

### Important Notes
- Use a 1kΩ resistor between Arduino Pin 8 and DFPlayer RX (recommended)
- Connect speaker directly to SPK_1 and SPK_2 pins (no amplifier needed for basic setup)
- Ensure common ground between Arduino and DFPlayer
- The module can handle 4Ω or 8Ω speakers

---

## SD Card Setup

### Format the SD Card
1. Use a Micro SD card (32GB max, 8GB recommended)
2. Format as **FAT32** (not exFAT)
3. Use default allocation size (4096 bytes)

### File Naming Convention
Audio files **must** be named with 4-digit numbers:
```
0001.mp3
0002.mp3
0003.mp3
...
0017.mp3
```

### Create Root MP3 Folder (Optional but Recommended)
Create a folder named `mp3` on the SD card and place all files there:
```
/mp3/0001.mp3
/mp3/0002.mp3
...
```

---

## Required Audio Files

Create or download the following 17 audio files and copy them to your SD card:

### System Sounds
| File | Sound | Description | Example Content |
|------|-------|-------------|-----------------|
| `0001.mp3` | Startup | System initialization | "System ready" or startup chime |
| `0017.mp3` | Acknowledged | Alert acknowledgment | "Alert acknowledged" or confirmation beep |

### Generic Alert Levels
| File | Sound | Description | Example Content |
|------|-------|-------------|-----------------|
| `0002.mp3` | Info | Informational alert | Single beep or "Information" |
| `0003.mp3` | Warning | Warning level | Double beep or "Warning" |
| `0004.mp3` | Critical | Critical level | Triple beep or "Critical alert" |
| `0005.mp3` | Danger | Danger level | Continuous alarm or "Danger! Immediate attention required" |

### Boost Pressure Alerts
| File | Sound | Description | Example Content |
|------|-------|-------------|-----------------|
| `0006.mp3` | Boost Warning | Boost exceeds warning threshold | "Boost pressure warning" |
| `0007.mp3` | Boost Critical | Boost exceeds critical threshold | "Boost pressure critical" |
| `0008.mp3` | Boost Danger | Severe overboost condition | "Overboost danger! Reduce throttle immediately!" |

### Intake Air Temperature Alerts
| File | Sound | Description | Example Content |
|------|-------|-------------|-----------------|
| `0009.mp3` | IAT Warning | Intake temp warning | "Intake air temperature warning" |
| `0010.mp3` | IAT Critical | Intake temp critical | "Intake air temperature critical" |

### Exhaust Gas Temperature Alerts
| File | Sound | Description | Example Content |
|------|-------|-------------|-----------------|
| `0011.mp3` | EGT Warning | Exhaust temp warning | "Exhaust temperature warning" |
| `0012.mp3` | EGT Critical | Exhaust temp critical | "Exhaust temperature critical" |
| `0013.mp3` | EGT Danger | Exhaust temp danger | "Exhaust temperature danger! Reduce load!" |

### Coolant Temperature Alerts
| File | Sound | Description | Example Content |
|------|-------|-------------|-----------------|
| `0014.mp3` | Coolant Warning | Coolant temp warning | "Coolant temperature warning" |
| `0015.mp3` | Coolant Critical | Coolant temp critical | "Coolant temperature critical" |
| `0016.mp3` | Coolant Danger | Coolant temp danger | "Coolant temperature danger! Stop engine!" |

---

## Audio File Creation

### Options for Creating Audio Files

#### Option 1: Text-to-Speech (TTS)
Use online TTS services to generate voice alerts:
- **Google TTS**: https://cloud.google.com/text-to-speech
- **Amazon Polly**: https://aws.amazon.com/polly/
- **Microsoft Azure TTS**: https://azure.microsoft.com/en-us/services/cognitive-services/text-to-speech/
- **Free alternatives**: Natural Reader, TTSFree, etc.

Recommended voice settings:
- Voice: Male, authoritative (for critical alerts)
- Speed: Normal to slightly slow
- Format: MP3, 16-bit, 22050 Hz or 44100 Hz

#### Option 2: Record Your Own
Use audio recording software:
- **Audacity** (free, open-source)
- **Adobe Audition**
- **GarageBand** (Mac)

Recording tips:
- Use a good quality microphone
- Record in a quiet environment
- Speak clearly and with appropriate urgency for each alert level
- Keep files short (1-3 seconds for beeps, 2-5 seconds for voice)

#### Option 3: Use Sound Effects
Download royalty-free sound effects:
- **Freesound**: https://freesound.org/
- **Zapsplat**: https://www.zapsplat.com/
- Search for: "warning beep", "alarm", "alert tone"

#### Option 4: Mix Both
Combine beeps with voice:
- Start with attention-grabbing beep
- Follow with voice message
- Example: [BEEP BEEP] + "Boost pressure critical"

### Audio Specifications
- **Format**: MP3
- **Sample Rate**: 22050 Hz, 44100 Hz, or 48000 Hz
- **Bit Rate**: 128 kbps or higher
- **Channels**: Mono or Stereo
- **Length**: 0.5 - 5 seconds (keep alerts concise)

---

## Alert Behavior

### Alert Levels and Repeat Intervals

The system automatically plays appropriate sounds based on sensor conditions:

| Alert Level | Sound Files Used | Repeat Interval | When Triggered |
|-------------|------------------|-----------------|----------------|
| **Info** | Generic (0002) | Every 5 seconds | Minor informational alerts |
| **Warning** | Parameter-specific (0006, 0009, 0011, 0014) or generic (0003) | Every 3 seconds | Warning thresholds exceeded |
| **Critical** | Parameter-specific (0007, 0010, 0012, 0015) or generic (0004) | Every 2 seconds | Critical thresholds exceeded |
| **Danger** | Parameter-specific (0008, 0013, 0016) or generic (0005) | Every 1 second | Dangerous conditions - immediate action required |

### Smart Sound Selection

The system intelligently selects sounds:
1. **First**: Tries to use parameter-specific sound (e.g., BOOST, EGT, COOLANT)
2. **Fallback**: Uses generic alert level sound if no specific sound exists
3. **Repeat**: Automatically repeats at appropriate intervals until acknowledged

### Example Scenarios

**Scenario 1: Boost Overboost**
- Boost reaches 21 PSI (danger threshold)
- System plays `0008.mp3` ("Overboost danger!")
- Repeats every 1 second until acknowledged or condition clears

**Scenario 2: High Exhaust Temperature**
- EGT reaches 610°C (warning threshold)
- System plays `0011.mp3` ("Exhaust temperature warning")
- Repeats every 3 seconds

**Scenario 3: Multiple Alerts**
- Both coolant and EGT are critical
- System plays the highest priority alert
- Shows combined alert message on display

---

## Configuration

### Volume Control

Default volume is set in `config.h`:
```cpp
#define SPEAKER_VOLUME  20  // Range: 0-30
```

You can also change volume at runtime:
```cpp
Alerts_SetVolume(25);  // Set volume to 25
```

### EQ Settings

Adjust the equalizer mode in `config.h`:
```cpp
#define SPEAKER_EQ_MODE  0
```

Available EQ modes:
- `0`: Normal
- `1`: Pop
- `2`: Rock
- `3`: Jazz
- `4`: Classic
- `5`: Bass

---

## Testing

### Initial Test Procedure

1. **Insert SD Card**: Ensure all 17 MP3 files are present
2. **Power On**: Arduino should play startup sound (0001.mp3)
3. **Check Serial Monitor**: Verify speaker module initialization
4. **Test Individual Sounds**: Use serial commands or modify code to test each file

### Serial Debug Output

Enable debug output to monitor speaker module:
```cpp
#define ENABLE_SERIAL_DEBUG  true
```

You should see:
```
Alerts: Initializing DFPlayer Mini speaker module...
Alerts: Speaker module initialized
  - Volume: 20
  - EQ Mode: 0
  - Files available: 17
Speaker: Playing sound 1
```

### Common Issues

#### No Sound Output
- **Check**: SD card is inserted correctly
- **Check**: Files are named correctly (0001.mp3, not 1.mp3)
- **Check**: SD card is formatted as FAT32
- **Check**: Speaker connections are correct
- **Check**: Volume is not set to 0

#### Module Not Detected
- **Check**: TX/RX wiring (should be crossed: Arduino TX → Module RX)
- **Check**: Power connections (5V and GND)
- **Check**: Use 1kΩ resistor on TX line if needed
- **Check**: DFPlayer module is not defective

#### Files Not Playing
- **Check**: File format is MP3 (not WAV, OGG, etc.)
- **Check**: Sample rate is supported (22050 Hz or 44100 Hz recommended)
- **Check**: SD card is not full
- **Check**: Files are in root directory or `/mp3/` folder

#### Poor Audio Quality
- **Increase**: Bit rate of MP3 files (use 192 kbps or higher)
- **Check**: Speaker impedance matches (4Ω or 8Ω)
- **Adjust**: EQ mode to suit speaker characteristics
- **Reduce**: Volume if distortion occurs at high levels

---

## Customization

### Adding More Sounds

To add additional sound files:

1. **Update config.h** with new sound definitions:
```cpp
#define SOUND_CUSTOM_ALERT  18
```

2. **Add MP3 file** to SD card:
```
0018.mp3
```

3. **Update alerts.cpp** to use new sound:
```cpp
case CUSTOM_CONDITION:
  playSpeaker(SOUND_CUSTOM_ALERT);
  break;
```

### Changing Alert Intervals

Modify repeat intervals in `config.h`:
```cpp
#define ALERT_REPEAT_WARNING   3000  // Change to 5000 for 5 seconds
#define ALERT_REPEAT_CRITICAL  2000  // Change to 3000 for 3 seconds
```

---

## Alternative Speaker Modules

While this system is designed for DFPlayer Mini, you can adapt it for:

### WTV020-SD
- Similar SD card-based module
- Simpler control protocol
- Requires code modifications

### JQ6500
- Built-in flash memory
- USB programmable
- Requires different library

### Grove MP3 v2.0
- Based on WT2003S chip
- I2C or Serial control
- Higher quality audio

---

## Troubleshooting Checklist

- [ ] SD card is formatted as FAT32
- [ ] All 17 MP3 files are present and correctly named
- [ ] Wiring matches diagram (TX/RX are crossed)
- [ ] 1kΩ resistor on TX line (recommended)
- [ ] Power connections are secure (5V and GND)
- [ ] Speaker is 3-8Ω impedance
- [ ] Speaker wires are connected to SPK_1 and SPK_2
- [ ] Serial monitor shows "Speaker module initialized"
- [ ] Volume is set between 10-25 (not 0)
- [ ] Audio files are MP3 format (not WAV, OGG, etc.)

---

## Resources

### Libraries Required
- **DFRobotDFPlayerMini**: Install via Arduino Library Manager
- **SoftwareSerial**: Built-in Arduino library

### Installation
```
Arduino IDE → Tools → Manage Libraries → Search "DFRobotDFPlayerMini" → Install
```

### Documentation
- DFPlayer Mini datasheet: [DFRobot Wiki](https://wiki.dfrobot.com/DFPlayer_Mini_SKU_DFR0299)
- Library documentation: [GitHub](https://github.com/DFRobot/DFRobotDFPlayerMini)

---

## Summary

The speaker module provides:
- ✅ Clear, verbal warnings for critical conditions
- ✅ Parameter-specific alerts (boost, temperature, coolant)
- ✅ Adjustable volume and EQ settings
- ✅ Easy customization with MP3 files
- ✅ Low cost (~$3-5 for module)
- ✅ Simple wiring (3 wires: TX, RX, GND)

This is a significant upgrade over a simple piezo buzzer, providing context-aware audio alerts that clearly communicate what parameter is out of range and how urgent the situation is.
