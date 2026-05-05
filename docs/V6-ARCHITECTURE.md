# AmbiSense v6 — Architecture (locked decisions)

Status: **active** as of 2026-05-05. This document captures decisions that
must not be relitigated without an explicit conversation. If you're picking
up the v6 rewrite cold, read this first.

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
| `ld2410`   | HiLink LD2410(B/C)| v1   | Single target; existing v5 hardware |
| `ld2412`   | HiLink LD2412    | v1    | Per-gate sensitivity                |
| `ld2420`   | HiLink LD2420    | v1    | Presence only (no distance)         |
| `ld2450`   | HiLink LD2450    | v1    | Up to 3 targets, x/y/speed          |
| `sim`      | Simulator        | v1    | Synthetic traces for desk testing   |

**One radar per device.** The C3 has only one usable UART beyond the
console, and the user explicitly confirmed this constraint. Multi-radar
fusion happens at the mesh level (each device contributes its sensor).

## Mesh model: peer mesh, NOT master/slave

Every device:

1. Has the **full topology config** in NVS (which segments exist, which
   device owns each segment, sensor positions in cm along the strip).
2. **Broadcasts its radar reading at 5 Hz** to all peers via ESP-NOW.
3. Runs the **same fusion algorithm locally** on the merged peer stream
   and arrives at the same global "active person position".
4. Renders **only its own LED segment** on its **own local strip**.

Properties:

- **No leader required for control.** Any device can drop and the others
  keep working at degraded fusion accuracy.
- **Web UI host = elected coordinator** (lowest-MAC peer). User browses
  to `ambisense.local`, mDNS resolves to whoever is elected. No "which
  IP do I open?" confusion.
- **Config gossip.** Any device accepts a config write; gossip propagates
  with a `config_version` stamp. Lex tiebreaker on MAC for simultaneous
  writes.

### Critical physical fact: each device has its OWN local strip

LED strips need power injection on both ends for long runs and the data
signal degrades over distance. We do **not** chain a single strip across
multiple devices. Each device drives its own physical strip; the "global
LED address space" is purely virtual:

```
device A (segment 0):   physical pixels 0..99   == virtual 0..99
device B (segment 1):   physical pixels 0..99   == virtual 100..199
device C (segment 2):   physical pixels 0..99   == virtual 200..299
```

When the active person is at virtual position 250:

- A renders background only (250 not in 0..99).
- B renders background only (250 not in 100..199).
- C renders the moving light at *local* pixel 50 (250 − 200).

This shrinks ESP-NOW bandwidth dramatically — peers exchange ~24-byte
`global_state_t` frames (active position + mode + color + effect tick),
not pixel-level commands. ~600 B/s mesh chatter for a 5-device install.

### Wire protocol (PR #4 will implement)

```c
typedef struct {              // ESP-NOW, broadcast every 200 ms
  uint8_t  msg_type;          // 1=TARGET, 2=HEARTBEAT, 3=CONFIG_GOSSIP, 4=CHAN_ANNOUNCE
  uint8_t  device_idx;
  uint16_t config_version;
  uint16_t distance_cm;
  int16_t  x_cm, y_cm;        // valid for LD2450, else 0
  int16_t  velocity_cms;
  uint8_t  energy;
  uint8_t  flags;             // bit0=primary_target_present, bit1=is_coordinator
  uint64_t ts_us;
} peer_msg_t;
```

Channel-follow-STA: when any device joins a router, it broadcasts
`CHAN_ANNOUNCE`; peers update via `esp_wifi_set_channel()`. Solves the
v5 "channel 6 forced, router on 11, mesh dies" silent failure.

Encrypted ESP-NOW: PMK/LMK pairing during a 3-second-button-hold
pairing window. Stops a neighbour with the same firmware from joining
your mesh accidentally.

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
    ├── board/                  # PR #1 ✓ — board profile struct + 4 profiles
    ├── settings/               # PR #1 ✓ — NVS facade replacing v5 EEPROM
    ├── status_led/             # PR #1 ✓ — pattern-driven LED in own task
    ├── netmgr/                 # PR #2  — Wi-Fi STA/AP + mDNS + captive DNS
    ├── webui/                  # PR #2  — esp_http_server + LittleFS + auth
    ├── ota/                    # PR #2  — esp_https_ota wrapper
    ├── radar/                  # PR #3  — driver registry (LD2410/2412/2420/2450/sim)
    ├── motion/                 # PR #3  — PI smoother (port of v5 algorithm)
    ├── led_engine/             # PR #3  — 11 modes via led_strip RMT
    ├── mesh/                   # PR #4  — ESP-NOW peer mesh
    └── topology/               # PR #4  — explicit L/U/asymmetric stair model
```

### FreeRTOS task model

| Task              | Pri | Stack | Period      | Responsibility                      |
| ----------------- | --- | ----- | ----------- | ----------------------------------- |
| `radar_task`      | 6   | 3 KB  | UART event  | Read radar bytes → `radar_frame_t`  |
| `motion_task`     | 5   | 4 KB  | 50 Hz       | PI smoother → publishes `target_t`  |
| `mesh_rx_task`    | 5   | 4 KB  | event       | ESP-NOW callbacks → `target_q`      |
| `mesh_tx_task`    | 4   | 3 KB  | 5 Hz        | Broadcast our reading; topology gossip |
| `led_render_task` | 4   | 6 KB  | 60 Hz       | Read fused target → framebuffer → `led_strip_refresh()` |
| `web_task`        | 3   | 6 KB  | event       | HTTPD handler thread                |
| `health_task`     | 2   | 2 KB  | 1 Hz        | Heap watch, peer timeout, status LED |

The single integration point is `target_q` — radar local + ESP-NOW
remote both push into it; motion fusion + LED render both consume it.
This collapses v5's four ad-hoc paths in `processRadarReading()`
(master / slave / standalone / no-slaves) into one uniform pipeline.

---

## NVS schema (replacing v5's 320-byte EEPROM map)

| Namespace | Keys (representative) |
| --------- | --------------------- |
| `sys`     | `device_name` |
| `board`   | `id`, `led_pin`, `radar_rx`, `radar_tx`, `button`, `status_led`, `radar_kind` |
| `led`     | `count`, `brightness`, `r/g/b`, `mode`, `span`, `center_shift`, `trail`, `dir_light`, `bg_mode`, `effect_speed`, `effect_intensity` |
| `dist`    | `min_cm`, `max_cm` |
| `motion`  | `enabled`, `pos_smooth`, `vel_smooth`, `predict`, `p_gain`, `i_gain` |
| `mesh`    | `peers` (blob: count + 6×N MAC array), `pmk`, `channel`, `priority_mode` |
| `topo`    | `kind` (straight/L/U/custom), `total_leds`, `segments` (blob) |
| `wifi`    | `ssid`, `pass` (encrypted w/ chip-derived key), `static_ip` (optional), `mdns_name` |
| `auth`    | `admin_pass_hash` (PBKDF2-SHA256, 250k rounds) |

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

New endpoints summary:

```
POST /api/auth/login         → cookie session
WS   /api/live               → distance + RSSI + heap @ 5 Hz, replaces XHR polling
GET  /api/board/profiles     → board dropdown
POST /api/board              → save board id + pin overrides; reboot
GET  /api/radar/kinds        → ld2410 | ld2412 | ld2420 | ld2450 | sim
POST /api/topology           → persist L/U/custom + per-segment LED ranges
POST /api/ota                → multipart firmware upload
POST /api/sim/trace          → simulator-driver: replay a recorded distance trace
GET  /api/version            → app version + git sha + idf version + free heap
GET  /api/logs               → ring buffer of recent ESP_LOG output
```

---

## Open decisions (locked for v6.0)

These were resolved by the user with "decide for me":

| Decision     | Choice                                                       |
| ------------ | ------------------------------------------------------------ |
| Boards in v1 | All four profiles ship; only C3 validated, others build-clean |
| Auth default | OFF; banner until configured; PBKDF2-SHA256 hash             |
| OTA signing  | Unsigned for v6.0; signed-OTA on the v6.x roadmap            |
| UI framework | Preact + Tailwind + Vite + TypeScript; bundle target <80 KB gz |
| Mesh model   | Peer mesh with elected coordinator (lowest-MAC)              |
| MQTT         | Off by default; HA auto-discovery format when enabled         |
| Repo strategy | Same repo; `legacy/v5-arduino` archive; `v6-idf-rewrite` working branch; tagged releases `v6.0.0-alpha.N` |

## What's NOT in v6.0 (deferred)

- BLE Mesh / Thread / Matter (would require C6/S3 only — rules out C3)
- Simultaneous LD2410 + LD2450 on one board (C3 has only one usable UART)
- Signed OTA (defer until update flow is exercised)
- Multi-room / cross-house mesh (≤5 nodes per install is the design point)
- Anonymous telemetry / phone-home (never)

These can be added later without breaking the architecture.
