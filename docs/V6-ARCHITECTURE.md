# AmbiSense v6 — Architecture (locked decisions)

Status: **v6.x — single-sensor architecture, PCB-lock candidate** as of
2026-05-07. This document captures decisions that must not be relitigated
without an explicit conversation. If you're picking up the v6 rewrite
cold, read this first.

> ## v6.x architectural pivot (2026-05-07)
>
> v6.0.0 shipped a dual-device ESP-NOW master/slave architecture. Bench
> testing on real U-shape and L-shape installs surfaced ESP-NOW
> round-trip jitter (5–100 ms with retries) that added visible LED lag,
> and the LD2450's (x, y, speed) target stream covers both arms of an L
> or U from a single sensor mounted at the inside corner. v6.x drops
> the dual-device architecture entirely and locks the firmware to one
> ESP32-C3 + one LD2450 per install, in preparation for a custom PCB.
>
> **Removed in v6.x:** `mesh` component, `topology` component, peer
> broadcast / coordinator election / pairing window / fusion modes,
> LD2410 / LD2412 / LD2420 radar drivers, the Mesh screen in the web UI,
> all `/api/mesh*` and `/api/topology*` endpoints, the `mesh` and `topo`
> NVS namespaces.
>
> **Kept:** ESP-IDF + FreeRTOS task model, the LD2450 driver, motion
> Kalman filter, OTA + rollback + captive portal + auth, the Preact web
> UI (six screens: Live, LEDs, Motion, Hardware, Network, System), the
> runtime board picker.
>
> The sections below are the v6.x truth. Sections that describe the
> dropped dual-device architecture have been rewritten in place; see
> the v6.0.0 release page for the historical version.

## Why v6 exists

v5.1.1 (Arduino) works but is structurally constrained:

- Every subsystem runs cooperatively in `loop()` — radar reads, LED
  rendering, HTTP serving, ESP-NOW. A slow web request stalls the LEDs;
  a heavy Fire-mode frame stalls radar reads.
- EEPROM byte-offset layout (320 bytes across 6 sections) — fragile, no
  versioning, no atomicity.
- One radar driver hard-coded; no path to LD2450 multi-target tracking.
- Master/slave ESP-NOW with implicit topology — equal LED-segment splits
  break asymmetric stairs (3-step landing + 12-step main flight).
- WiFi channel pinned to 6 even when the master joins a router on
  channel 11 — silent failure.
- No OTA, no captive portal, no auth, plaintext WiFi creds in EEPROM.

v6 is an ESP-IDF + FreeRTOS rewrite that fixes all of the above while
preserving the v5 visual modes, the user-facing API surface (most
endpoints kept compatible), and the master-slave-style coordination
behaviour for U/L stair installs.

---

## Target hardware

| Profile               | Validated | Default LED pin | Default radar pins |
| --------------------- | --------- | --------------- | ------------------ |
| `esp32c3-supermini`   | ✅ yes    | GPIO 10         | RX 20 / TX 21      |
| `esp32-devkit`        | builds    | GPIO 5          | RX 16 / TX 17      |
| `esp32s3-zero`        | builds    | GPIO 21         | RX 4  / TX 5       |
| `esp32c6-devkit`      | builds    | GPIO 8          | RX 4  / TX 5       |

Profiles ship in `firmware/components/board/board.c`. Each declares an
`unsafe_pin_mask` covering strapping pins, USB-Serial-JTAG D-/D+, and
internal SPI flash. The web UI (PR #5) will refuse pin remaps that fall
on unsafe pins.

The C3 SuperMini is the primary target for v6.0. Other boards are
"compile-clean" until the user gets hardware in hand.

## Target sensors (radar abstraction layer)

A driver registry compiled into `components/radar/`. All drivers linked
in; one selected at runtime via the `board.radar_kind` NVS key. Adding a
new sensor = +1 file + 1 dropdown entry, no firmware reflash needed.

| Driver id  | Sensor           | Tier  | Notes                               |
| ---------- | ---------------- | ----- | ----------------------------------- |
| `ld2450`   | HiLink LD2450    | v6.x  | Up to 3 targets, x/y/speed (24 GHz) |
| `sim`      | Simulator        | v6.x  | Synthetic traces for desk testing   |

**LD2450-only on shipping hardware.** The (x, y, speed) target stream
covers straight / L-shape / U-shape geometries from a single sensor
mounted at the inside corner of the LED run. The v6.0 LD2410 / 2412 /
2420 drivers were removed in v6.x — they only emit single-target
distance and were intended for the dual-device "two single-target
sensors fusing into one LED span" model that v6.x dropped.

**One radar per device.** The C3 has only one usable UART beyond the
console. With dual-device removed there is no longer a need to
multi-radar fuse at all.

## Topology model (v6.x): single sensor, single device

One ESP32-C3 reads one LD2450 over UART, smooths the target stream
through the motion Kalman filter, and drives one WS2812 strip. There
is no peer broadcast, no coordinator election, no pairing flow, no
topology gossip.

### How a single LD2450 covers L-shape and U-shape geometries

The LD2450 reports up to three concurrent targets as `(x_mm, y_mm,
v_mm/s)` within a 60° horizontal cone, ~6 m range. Mounted at the
inside corner of an L or the centre-back of a U, both arms of the LED
run sit inside that cone. The LED engine maps the chosen target's
position to a virtual LED index using the strip-shape configuration
saved in NVS — no need for two sensors comparing notes.

Mounting constraint to document for installers: the sensor must have
line-of-sight to both arms of the LED run. For long arms (>5 m), test
detection at the far end before locking a kit configuration; the
LD2450's effective range degrades with off-axis targets.

### Why dual-device was removed

v6.0 implemented a dual-device peer mesh (every device broadcast its
local radar reading on ESP-NOW at 5 Hz, ran identical fusion locally,
and rendered its own LED segment of the global virtual address space).
On real U-shape and L-shape benches:

- ESP-NOW round-trip latency was 5–15 ms in best case but spiked to
  50–100 ms under retry, adding visible LED lag the motion filter
  could not hide.
- Pairing flow added significant firmware surface (mesh component,
  topology component, ~5 HTTP endpoints, the Mesh screen) and
  install-time friction (which device is the master? did pairing
  complete? which IP do I open?).
- The LD2450's x/y stream made the second sensor redundant for the
  shapes most users actually install (L-shape stairs and U-shape
  hallways).

The trade-off accepted in v6.x: installers must mount the sensor at
the geometric vantage point of the LED run rather than anywhere
convenient on either arm. Acceptable for a kit product with documented
mounting guidance.

---

## Software structure

```
firmware/
├── CMakeLists.txt              # IDF project root
├── partitions.csv              # NVS + 2x OTA + LittleFS + coredump
├── sdkconfig.defaults          # Common knobs (WDT, brownout, coredump, …)
├── sdkconfig.defaults.esp32c3  # C3-specific (USB-Serial-JTAG console, brownout)
├── main/main.c                 # app_main: bring up tasks; no business logic
└── components/
    ├── board/                  # board profile struct + 4 profiles
    ├── settings/               # NVS facade replacing v5 EEPROM
    ├── status_led/             # pattern-driven LED in its own task
    ├── button/                 # BOOT-button polling (long-press reserved for v6.1 factory reset)
    ├── auth/                   # PBKDF2-SHA256 admin password
    ├── netmgr/                 # Wi-Fi STA/AP + mDNS + captive DNS
    ├── webui/                  # esp_http_server + embedded UI bundle
    ├── ota/                    # esp_https_ota wrapper + rollback
    ├── radar/                  # driver registry (LD2450 + sim)
    ├── motion/                 # Kalman smoother
    └── led_engine/             # 11 modes via led_strip RMT
```

`mesh/` and `topology/` were deleted in the v6.x rewrite.

### FreeRTOS task model

| Task              | Pri | Stack | Period      | Responsibility                      |
| ----------------- | --- | ----- | ----------- | ----------------------------------- |
| `radar_task`      | 6   | 4 KB  | UART event  | Read radar bytes → `radar_frame_t`  |
| `motion_task`     | 5   | 4 KB  | 50 Hz       | Kalman smoother → publishes `target_t` |
| `led_render_task` | 4   | 6 KB  | 60 Hz       | Read smoothed target → framebuffer → `led_strip_refresh()` |
| `web_task`        | 3   | 8 KB  | event       | HTTPD handler thread                |
| `tele_pump`       | 3   | 3 KB  | 20 Hz       | Publish smoothed target to /api/live WS clients |
| `ws_bcast`        | 3   | 4 KB  | 20 Hz       | Coalesce + emit live JSON to WS    |
| `status_led_task` | 2   | 2 KB  | pattern     | Drive onboard LED blink pattern    |

The pipeline is linear: radar → motion → led_engine. No fan-in queue
needed; each stage owns its smoothed/rendered state and the next stage
pulls.

---

## NVS schema (replacing v5's 320-byte EEPROM map)

| Namespace | Keys (representative) |
| --------- | --------------------- |
| `sys`     | `device_name` |
| `board`   | `id`, `led_pin`, `radar_rx`, `radar_tx`, `button`, `status_led`, `radar_kind` |
| `led`     | `count`, `brightness`, `r/g/b`, `mode`, `span`, `center_shift`, `trail`, `dir_light`, `bg_mode`, `effect_speed`, `effect_intensity` |
| `dist`    | `min_cm`, `max_cm` |
| `motion`  | `mode`, `enabled`, `response`, `look_ahead_ms`, `outlier_strength`, `pos_smooth`, `vel_smooth`, `predict`, `p_gain`, `i_gain` |
| `wifi`    | `ssid`, `pass`, `hostname`, `static_ip` (optional) |
| `auth`    | `admin_pass_hash` (PBKDF2-SHA256, 250k rounds) |

The `mesh` and `topo` namespaces were removed in v6.x. Devices upgraded
from v6.0 may still have orphan keys in those namespaces; the firmware
never reads them.

NVS is journaled (atomic per-key writes), wear-levelled, typed (no
manual hi/lo byte unpacking), and versionable. Replaces v5's manual
XOR-CRC sectioned layout entirely.

No migration shim from v5 EEPROM — the user explicitly confirmed v6 is
a clean cutover (no backwards compatibility with Arduino fleet).

---

## HTTP API

The PR #1 design preserves every existing v5 endpoint (compatibility for
external integrations like Home Assistant), and adds the missing v6
surface. See `docs/V6-ROADMAP.md` for the per-PR endpoint deliveries.

v6.x endpoint summary:

```
POST /api/auth/login         → cookie session
WS   /api/live               → distance + raw + RSSI + heap @ 20 Hz
GET  /api/board/profiles     → board dropdown
POST /api/board              → save board id + pin overrides; reboot
GET  /api/radar/kinds        → ld2450 | sim
GET  /api/radar/diag         → driver id, byte/frame counters, last-frame age
GET  /api/settings           → flat read of every NVS namespace
POST /api/settings           → batched write
POST /api/ota                → application/octet-stream firmware upload
GET  /api/version            → app version + git sha + idf version + target
```

Removed in v6.x: `/api/mesh`, `/api/mesh/identify`, `/api/topology`.

---

## Open decisions (locked for v6.0)

These were resolved by the user with "decide for me":

| Decision     | Choice                                                       |
| ------------ | ------------------------------------------------------------ |
| Boards in v6.x | C3 SuperMini is the PCB-rev target; other profiles still build-clean |
| Sensor       | LD2450 only on shipping hardware (sim retained for desk testing) |
| Architecture | Single-sensor / single-device — no peer mesh                  |
| Auth default | OFF; banner until configured; PBKDF2-SHA256 hash             |
| OTA signing  | Unsigned; signed-OTA on the v6.x roadmap                      |
| UI framework | Preact + Vite + TypeScript; single inlined HTML bundle (~25 KB gz) |
| MQTT         | Off by default; HA auto-discovery format when enabled         |
| Repo strategy | Same repo; `legacy/v5-arduino` archive; `v6-idf-rewrite` working branch |

## What's NOT in v6.x (deferred)

- Multi-device pairing in any form (removed; do not relitigate without
  a different sensor topology)
- BLE Mesh / Thread / Matter
- Simultaneous LD2410 + LD2450 on one board (C3 has only one usable UART)
- Signed OTA (defer until update flow is exercised)
- Anonymous telemetry / phone-home (never)
