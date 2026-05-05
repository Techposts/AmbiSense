# AmbiSense v6 — Hardware Setup & Troubleshooting

For locked architectural decisions about which boards/sensors are
supported, see [`V6-ARCHITECTURE.md`](V6-ARCHITECTURE.md). This file
covers the practical "I have hardware in front of me" stuff.

## Board recommendation (read this first)

| Tier | Board | Why |
| ---- | ----- | --- |
| **Recommended** | **ESP32-S3 DevKitC-1** *or* **ESP32-S3-Zero** | Dual-core LX7 @ 240 MHz, native USB-OTG (no CH340 driver fights), 512 KB SRAM. Wi-Fi/HTTP can be pinned to core 0 and radar+LED+motion to core 1 — slider floods can't starve the render loop. Headroom for v6.1 features (auto-topology, encrypted ESP-NOW, larger strips). Best choice for a stairwell with mesh peers. |
| **Supported** | **ESP32-C3 SuperMini** | Single-core RISC-V @ 160 MHz. The validated v6.0 reference build. Fine for **single-strip, single-device** installs. The 300 ms client-side debounced-save shipped with v6.0 prevents HTTP saturation under slider drag. Stops being the right choice the moment you want >2 mesh peers or >300 LEDs. |
| **Deprecated** | ESP32 classic (WROOM-32) | Older silicon, no native USB, no advantage over S3. Profile still builds; not recommended for new installs. |
| **Avoid** | ESP32-C6 | Single-core, *less* SRAM than C3, and Wi-Fi 6 / Thread don't help AmbiSense. |

If you're starting from scratch and asked us "which board do I buy" → **S3-Zero**. If you already have a C3 SuperMini on the bench → it works.

## Reference wiring (ESP32-C3 SuperMini)

This is the validated reference build for v6.0.

| Function       | C3 GPIO | Note                                |
| -------------- | ------- | ----------------------------------- |
| LED data (DIN) | GPIO 10 | WS2812 / NeoPixel                   |
| Radar RX (MCU) | GPIO 20 | from radar's TX                     |
| Radar TX (MCU) | GPIO 21 | to radar's RX                       |
| Reset / mode   | GPIO 4  | momentary button to GND             |
| Status LED     | GPIO 8  | onboard (active-low, on most clones)|
| 5 V power      | 5V pin  | for radar VCC                       |
| Ground         | GND     | radar, LED logic ground             |

**Pin defaults match the firmware out of the box** — flash the C3
SuperMini build, wire to spec, no NVS pin override needed. If you
remap pins from the web UI later, the unsafe-pin guard refuses
strapping/USB-JTAG/flash pins (GPIOs 9, 11–19 on C3).

### Power supply

- **Logic**: USB power is fine for radar + MCU + onboard status LED.
- **LED strip**: drive from a separate 5 V PSU sized to the strip
  (60 mA per WS2812 LED at full white). **Inject power on both ends**
  for runs above ~50 LEDs to prevent voltage droop and rainbow
  desaturation. The C3's 3.3 V LDO cannot power more than ~30 LEDs at
  full brightness — don't try.
- **Common ground**: tie the LED PSU's GND to the C3's GND so the
  data signal references correctly.

## Sensor reference wiring

Both LD2410 and LD2450 use 256000 baud UART, 5 V VCC, identical pinout:

```
Sensor       MCU (C3 SuperMini)
------       ------------------
TX     →     GPIO 20 (radar_rx)
RX     ←     GPIO 21 (radar_tx)
VCC    ←     5V
GND    ↔     GND
```

LD2450 adds OUT pin (digital presence indicator) — leave unconnected
for v6, the firmware reads everything via UART.

## Supported boards (v6.0)

| Profile               | Status        | LED pin | Radar RX | Radar TX | Button | Status LED    |
| --------------------- | ------------- | ------- | -------- | -------- | ------ | ------------- |
| `esp32s3-zero`        | recommended   | 21      | 4        | 5        | 9      | 21            |
| `esp32c3-supermini`   | ✅ validated  | 10      | 20       | 21       | 4      | 8 (active-low)|
| `esp32-devkit`        | deprecated    | 5       | 16       | 17       | 4      | 2             |
| `esp32c6-devkit`      | builds only   | 8       | 4        | 5        | 9      | 15            |

The `esp32s3-zero` profile is structurally validated (the firmware
builds and the pinmap is correct for the AliExpress S3-Zero / S3-Mini
clones) but Ravi's bench currently only has C3s — full hardware
validation pending arrival of S3 units.

Profiles defined in
[`firmware/components/board/board.c`](../firmware/components/board/board.c).
Adding a new board = +1 entry there + a build target in
`.github/workflows/firmware.yml`.

## Supported sensors (v6.0)

| Driver id  | Sensor          | Targets | x/y? |
| ---------- | --------------- | ------- | ---- |
| `ld2410`   | HiLink LD2410(B/C) | 1    | no   |
| `ld2412`   | HiLink LD2412   | 1       | no   |
| `ld2420`   | HiLink LD2420   | 1 (presence only) | no |
| `ld2450`   | HiLink LD2450   | up to 3 | yes (LD2450 only) |
| `sim`      | Synthetic       | scripted | optional |

Switch sensors at runtime via the web UI without reflashing — the
driver registry compiles every driver in and selects one from NVS at
boot.

## Troubleshooting

### Flash fails with "Failed to connect to ESP32-C3: No serial data received"

**Symptom**: `idf.py flash` fails repeatedly. The chip enumerates as
`/dev/cu.usbmodem*` (so USB-CDC is up) but esptool's SYNC packets
go unanswered. Sometimes the chip feels warm.

**Diagnosis ladder** — try in order:

1. **Check the port isn't being held by another process.**
   ```sh
   lsof /dev/cu.usbmodem*
   ```
   Common culprits on macOS: `LG Calibration` (LG monitor calibration
   daemon), Arduino IDE serial monitor, VSCode serial monitor. Kill
   any holders before retrying.

2. **Force the C3 into ROM download mode.** On most C3 SuperMini
   clones with two surface-mount buttons (RST + BOOT):
   - Hold `BOOT`.
   - While holding `BOOT`, tap `RST`.
   - Wait 1 s. Release `BOOT`.
   - Retry `idf.py flash`.

3. **Hold BOOT throughout the flash.** Some clones need `BOOT` held
   continuously, not just sampled at reset. Run `idf.py flash` with
   `BOOT` still pressed; release only after flash completes.

4. **For single-button boards**: `BOOT` is often a tiny solder pad on
   the back, or `BOOT = GPIO 9`. Bridge GPIO 9 to GND with tweezers
   while pressing `RST`.

5. **`--before usb_reset`** (the C3's USB-Serial-JTAG-specific reset).
   ```sh
   python -m esptool --chip esp32c3 -p /dev/cu.usbmodem... \
     -b 460800 --before usb_reset --after hard_reset \
     write_flash @flash_args
   ```

6. **macOS USB-CDC stuck state — restart the Mac.** This is the known
   final-resort fix when the chip enumerates but esptool can't sync
   despite all of the above. macOS sometimes caches a stale USB-CDC
   endpoint state for the C3's USB-Serial-JTAG and won't release it
   until reboot. Confirmed by Ravi as the working recovery on
   2026-05-05.

### Onboard LED stays solid after boot

Expected during PR #1 — `app_main` finishes setup and switches to
`STATUS_LED_AP_MODE` (slow 1 Hz blink). If it stays solid, you're on
PR #1's `BOOT` pattern still, which means `app_main` crashed before
reaching the `set_pattern(AP_MODE)` call. Check the serial log over
USB-Serial-JTAG (the C3's onboard USB IS the serial console).

### Where do I see logs?

The C3 SuperMini routes ESP-IDF console over its built-in USB-Serial-
JTAG peripheral. Connect via:

```sh
. $IDF_PATH/export.sh
idf.py -p /dev/cu.usbmodem... monitor
```

or any serial terminal at 115200 baud. Logs are also mirrored to a
16 KB in-RAM ring buffer accessible at `GET /api/logs` once PR #2's
web server is up.

### Device boots but second one isn't visible (no AP, no STA)

**Symptom**: After flashing, the device serial output shows boot logs
but no AP `AmbiSense-XXXX` shows up on a phone scan and the device
never joins Wi-Fi. First device works fine.

**Most common cause**: stale NVS holding a board profile for a
different MCU (e.g., `esp32-devkit` saved on a C3 from a prior test
flash). v6.0 ships an MCU-mismatch boot guard that catches this on
new boots, but if you're upgrading from an alpha build the safest
recovery is:

```sh
idf.py -p /dev/cu.usbmodemXXXXXXX erase-flash
idf.py -p /dev/cu.usbmodemXXXXXXX flash
```

The erase-flash wipes the NVS partition so the device boots with
factory defaults. Confirm with `idf.py monitor` — you should see
`Board profile (default): esp32c3-supermini (ESP32-C3 SuperMini)`
followed by netmgr starting AP.

### Slider in web UI throws ERR_CONNECTION_RESET

**Symptom**: Dragging a slider in the LED or Motion screen flood-fires
`POST /api/settings` and the browser console fills with
`ERR_CONNECTION_RESET` / `ERR_EMPTY_RESPONSE`.

**Cause**: The C3 is single-core. The HTTP server, ESP-NOW broadcast,
radar UART, motion smoother, and LED render task all run on one
RISC-V core. Sliders fire `onChange` ~30 times/second; each one was
posting + reloading at full rate, saturating the httpd's
`max_open_sockets = 7`.

**Fix**: v6.0 adds a 300 ms debounced-save on the client side
(`useDebouncedSave` in `frontend/src/screens.tsx`). The slider only
POSTs once you stop dragging, and multiple slider changes within
300 ms get coalesced into a single batched request body. If you
build the UI yourself, make sure you're on commit `0b63b6e` or later.

### Brownout reset loop

The C3's brownout detector is configured at threshold level 7 (~2.7 V).
Brownouts on a powered-only-by-USB C3 driving a long LED strip from
the same rail are common. v6 expects the **LED strip on its own PSU**;
running them off the C3's 5 V/3.3 V rails will trigger the brownout
detector and reboot you in a loop.

If you must run a few LEDs from USB power for testing, keep `count ≤ 30`
and `brightness ≤ 100/255`.
