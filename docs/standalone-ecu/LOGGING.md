# Complete logging and upload

The ECU records **everything, every drive**, and makes the logs easy to get
off the truck for analysis anywhere: a laptop, Claude, your own server, or
cloud storage. No AI runs on the board. Improved maps come back
through the app later (see "Maps coming back" below, not built yet).

```
STM32 (engine) --2 Mbaud UART--> ESP32-S3 --> microSD  /drives/drive_00012.jlog
                                          \-> Wi-Fi    junkr.local: list, download, live, auto-upload
                                                         |
                                tools/junkrlog.py  <-----+  -> CSV / Parquet / pandas
```

Nothing here changes the rev A hardware: the ESP32, its UART link to the
STM32 and the microSD slot are already on the board.

## What gets logged

| Record | Rate | Contents |
|---|---|---|
| SAMPLE | 100 Hz | 38 channels, listed below |
| INJ | every injection (each 180 crank deg) | segment count, crank period for that segment, planned Q and SPV window |
| EVENT | on change | boot, mode change, DTC set/clear, sync loss, ARM / SAFE_OK change, takeover refused, dropped frames, link up/down, loop overrun |
| META | every second | firmware version, calibration CRC-32 and size, board |
| SCHEMA | every second | the channel list itself (name, unit, wire type, scale) |
| FILE | top of each file | drive name, ESP32 firmware, wall-clock time when known |

The per-injection crank period is one value per cylinder firing, so
cylinder balance and smoothness can be worked out afterwards
(`cylinder_slot` = segment % 4; which physical cylinder slot 0 is gets
fixed on the car).

A simulated drive comes to about 13 kB/s, so **about 47 MB per hour of
driving**. A 32 GB card holds roughly 680 hours. When the card runs low,
the oldest drives that have already been uploaded are deleted. Drives
that haven't been uploaded are never deleted.

### Sample channels (format version 1)

| Channel | Unit | Notes |
|---|---|---|
| rpm, rpm_inst | rpm | per-segment and per-tooth |
| q | mm3/stroke | commanded fuel |
| spv_close, spv_open | NE deg | planned spill valve window |
| adv, adv_target, adv_err | deg BTDC | measured injection timing, target, error |
| tcv_duty | % | timing control valve |
| pedal, va_v | %, V | accelerator |
| map, map_v | kPa, V | manifold pressure |
| ect, ect_v | C, V | coolant |
| vbatt | V | |
| speed | km/h | |
| range, gear, mode | enum | mode 0 shadow, 1 takeover |
| flags | bits | synced, rpm_valid, inject, takeover, arm_req, arm_sw, supervisor_ok, arm, idl, brake, starter, od_sw, s1, s2, sl, svr, timing_open_loop, tdc_valid, standalone, timing_fault, egt_limit, wb_valid, wb_learn |
| dtc | bits | ne, timing, pedal, overspeed, ect, map, vbatt, supervisor, egt, wideband |
| sync_losses, agree_run, segments | count | |
| stock_tcv_duty | % | stock ECU's TCV, in shadow mode |
| mux0..2 | bits | every slow switch input, raw |
| loop_us | us | control loop time (CPU headroom) |
| egt, egt_v | C, V | exhaust temperature before the turbo, raw volts ([EXHAUST.md](EXHAUST.md)) |
| lambda, wb_v | -, V | wideband lambda, raw volts |
| egt_scale, rich_cut, smoke_trim | % | EGT fuel-limit allowance, rich-floor pull, live smoke-map trim |
| q_limit | mm3/stroke | final fuel limit after all of the above |

Channels are added by appending to `CHANNELS` in `shared/ecu_log.h`. Old
decoders keep working because every log carries its own schema.

## Getting logs off the truck

1. **Phone or laptop in the truck:** join the `JUNKR-ECU` Wi-Fi network and
   open `http://junkr.local` (or `http://192.168.4.1`). Tap a drive to
   download it. The page also shows live values.
2. **At home, automatically:** put your home Wi-Fi in `/junkr.json` on the
   card. When the truck can see it, the ECU joins, and (if `upload_url` is
   set) uploads every finished drive, one at a time, then records it in
   `/uploaded.txt`.
3. **From a computer on the same network:**
   `python3 tools/junkrlog.py fetch http://junkr.local logs/` downloads every
   drive you don't already have.
4. **Fallback:** pull the microSD card. Files are in `/drives/`.

### /junkr.json

Written as a template on first boot with a blank card:

```json
{
  "ap_ssid": "JUNKR-ECU",
  "ap_pass": "junkr-kzj95",
  "wifi_ssid": "your home network",
  "wifi_pass": "its password",
  "upload_url": "https://your-server.example/junkr/upload",
  "upload_token": "optional bearer token",
  "hostname": "junkr"
}
```

Change `ap_pass` before the truck goes anywhere: anyone on that network
can download logs and send the shadow/takeover request (the STM32 still
enforces every takeover condition itself).

**Upload endpoint:** each drive is one `POST` with the raw `.jlog` as the
body, the name in `X-Junkr-Drive`, and `Authorization: Bearer <token>` if a
token is set. Any 2xx response marks it uploaded. A Home Assistant
webhook, a 20-line Flask app on a Raspberry Pi, or a cloud function in
front of S3, R2 or Google Drive all work.

### HTTP API (port 80)

| Request | Returns |
|---|---|
| `GET /` | drive list and live view page |
| `GET /api/drives` | `{"drives":[{"name","size","uploaded"}],"recording","free","link","frames_good","frames_bad","ring_overflow"}` |
| `GET /drives/<name>` | the file (the drive being recorded can be downloaded too, up to the last flush) |
| `DELETE /drives/<name>` | deletes a finished drive |
| `GET /api/live` | latest sample, every channel by name |
| `POST /api/mode?m=S` / `m=T` | shadow, or request takeover (the STM32 decides) |

## Reading the logs

```
python3 tools/junkrlog.py info    drive_00012.jlog   # summary, channel list, events
python3 tools/junkrlog.py csv     drive_00012.jlog   # -> drive_00012/samples.csv, injections.csv, events.csv, channels.json
python3 tools/junkrlog.py parquet drive_00012.jlog   # needs pandas + pyarrow
```

Or from Python: `junkrlog.read(path).to_pandas()`. The CSV opens directly
in MegaLogViewer, a spreadsheet, or Grafana, and can be handed to Claude
along with this page for analysis.

## Drive files

- A new file starts at every power-up, and after the engine has been off
  for 10 minutes and starts again. Files are capped at 512 MB.
- Each file starts with a FILE frame, then the latest SCHEMA and META, so
  every file decodes on its own.
- The ESP32 flushes to the card every 2 s, so pulling power loses at most
  the last 2 s.
- If the card stalls, a 512 KB PSRAM buffer holds about 45 s.
- If the UART can't keep up, the STM32 drops frames rather than delay the
  control loop, and reports how many in a `dropped` event.

## Format (jlog v1)

```
A5 5A | type u8 | len u16 | payload[len] | crc16
```

All little-endian. The CRC is CRC-16/CCITT-FALSE over type, len and
payload. A reader that hits a bad frame scans forward for the next `A5 5A`,
so a corrupt byte costs one frame, not the file. The exact layouts are in
`shared/ecu_log.h`, which the STM32, the ESP32 and the host tests all
compile.

## Tests

- `pio test -e native -f test_log`: frame and CRC round trips, resync after
  garbage, sample packing and clamping, and a 9.5 s simulated drive (cold
  start, idle, 40 % pedal, release) through the real `Telemetry` code. It
  checks 100 Hz samples, no missing injection records, rpm following the
  engine, and that a slow link drops frames without blocking.
  `JLOG_OUT=test/fixtures/sim_drive.jlog` writes that drive to a file.
- `python3 tools/test_junkrlog.py`: the Python decoder reads that same
  firmware-written file, and survives deliberate corruption.

## Still to do

- **Maps coming back:** a calibration upload over the same link, with the
  STM32 checking every cell against hard limits, two map slots and
  one-tap revert. The log already records the calibration CRC, so every
  drive says which map it ran.
- Verify the 2 Mbaud link and SD write rate on the real board
  (BENCH_TEST.md).
- Add the CAN channels (hub, gauge, CAN wideband) to the log when those
  nodes exist. The analog EGT and wideband on AUX are already logged.
