# AmbiSense v6 — PR-by-PR roadmap

> **Status (2026-05-07): v6.0.0 released; v6.x architectural pivot in
> progress on `v6-idf-rewrite`.** v6.x drops the dual-device ESP-NOW
> master/slave architecture in favour of a single-sensor LD2450 design,
> in preparation for a custom PCB. The historical PR record below
> describes how v6.0.0 was built; the section "v6.x roadmap" at the
> bottom captures what changed after release and what's next.
>
> See [`V6-ARCHITECTURE.md`](V6-ARCHITECTURE.md) for the locked v6.x
> architectural decisions, and the
> [v6.0.0 release page](https://github.com/Techposts/AmbiSense/releases/tag/v6.0.0)
> for the historical user-facing summary.

Five PRs. Each is independently flashable and validated on real C3
hardware before the next starts. Releases tagged `v6.0.0-alpha.N` from
PR #2 onwards; final `v6.0.0` ships with PR #5.

For locked architectural decisions see [`V6-ARCHITECTURE.md`](V6-ARCHITECTURE.md).

---

## PR #1 — IDF skeleton

**Status: ✅ MERGED on `v6-idf-rewrite`, pushed to origin (2026-05-05).**

Delivered:
- Repo housekeeping: legacy v5 moved under `legacy/AmbiSense/`,
  branches `legacy/v5-arduino` and `v6-idf-rewrite` created.
- ESP-IDF v5.3-compatible project scaffold under `firmware/`.
- Custom partition table: 16 KB NVS + 8 KB OTA data + 1408 KB × 2 OTA
  app slots + 960 KB LittleFS + 64 KB coredump (4 MB total).
- `sdkconfig.defaults` with task watchdog, brownout detector,
  bootloader rollback, coredump-to-flash, esp_http_server WS support.
- `components/board/` — profile struct + 4 ship-ready profiles
  (`esp32c3-supermini` validated; others build-clean, untested).
  `unsafe_pin_mask` blocks UI pin remap to strapping/USB/flash GPIOs.
- `components/settings/` — NVS facade replacing v5's 320-byte EEPROM
  layout. PR #1 only exposes the `board` namespace; other namespaces
  populated as their owners come online.
- `components/status_led/` — pattern-driven LED in its own FreeRTOS task.
  Six patterns (BOOT, AP_MODE, STA_MODE, OTA, ERROR, PANIC) cover every
  user-visible state.
- `main/main.c` — resolves board profile (NVS override → compile-time
  default), applies per-pin NVS overrides while rejecting unsafe pins,
  spawns status_led task.
- `frontend/design-source/` — full Claude Design handoff bundle
  preserved for PR #5 to build against.
- `.github/workflows/firmware.yml` — IDF build matrix across all four
  targets on every push/PR; uploads tagged firmware artifacts.

**Build numbers** (C3 target):
- `ambisense.bin`: 0x30CF0 bytes (~195 KB)
- App slot free: 0x12F310 bytes (86%)
- Bootloader free: 35%

**Hardware validation status**: blocked on a one-off macOS USB-CDC
enumeration issue (the chip enumerates but esptool can't sync). Not a
firmware bug — see [`HARDWARE.md`](HARDWARE.md). Will be flashed once
the user retries after a Mac restart.

---

## PR #2 — Wi-Fi, web shell, OTA, auth scaffold

**Status: not started.**

Scope:
- `components/netmgr/` — Wi-Fi STA with fallback to AP, mDNS hostname,
  captive-portal DNS responder for AP mode (auto-pop on iOS/Android/
  Win11), Wi-Fi event loop integration.
- `components/webui/` — esp_http_server, LittleFS-served static UI
  (placeholder bundle), JSON settings round-trip for every NVS key,
  cookie-based auth (off by default, PBKDF2-SHA256 password hash),
  WebSocket endpoint stub at `/api/live`.
- `components/ota/` — `esp_https_ota` wrapper, `POST /api/ota` multipart
  upload, two-stage commit (write → verify → mark valid → reboot).
- Status LED transitions: BOOT → AP_MODE while waiting for credentials,
  STA_MODE heartbeat once connected.
- `GET /api/version`, `GET /api/logs` (ring buffer mirror of `ESP_LOG*`).

**Done criterion**: from a stock C3, connect phone to `AmbiSense-XXXX`
AP, captive portal pops, configure home Wi-Fi, device reboots into STA
mode, browse to `http://ambisense.local`, see placeholder dashboard,
upload a `.bin` firmware via OTA, device reboots into new firmware.

**Tag on completion**: `v6.0.0-alpha.1`.

---

## PR #3 — Radar driver registry, motion, LED engine

**Status: not started.**

Scope:
- `components/radar/` — driver registry pattern with one driver per
  sensor: `ld2410.c`, `ld2412.c`, `ld2420.c`, `ld2450.c`, `sim.c`.
  Selected at runtime via `board.radar_kind` NVS key. Common
  `radar_frame_t` API; LD2450's x/y/multi-target fields ignored by
  drivers that don't provide them.
- `components/motion/` — port of v5's PI smoother
  (`legacy/AmbiSense/radar_manager.cpp:38-198`). Same algorithm, runs
  in its own task at 50 Hz, output published to `target_q`.
- `components/led_engine/` — port of all 11 visual modes
  (`legacy/AmbiSense/led_controller.cpp` lines 273–650). RMT-backed
  `led_strip` driver replaces `Adafruit_NeoPixel`. Render task at
  60 Hz consumes `target_q`, writes framebuffer, refreshes strip
  asynchronously (non-blocking, unlike v5's `strip.show()`).
- `POST /api/sim/trace` — accept JSON-encoded distance traces for the
  simulator driver. Lets you debug LED modes without standing in front
  of a sensor.
- UI placeholder gets the LED preview canvas wired to live state.

**Done criterion**: standalone C3 + LD2450 + 30 LED test strip mimics
v5 visual behaviour at parity for all 11 modes; web request load no
longer glitches LED rendering; simulator replays a recorded trace
identically to a live one.

**Tag on completion**: `v6.0.0-alpha.2`.

---

## PR #4 — Peer mesh, topology, ESP-NOW

**Status: not started.**

Scope:
- `components/mesh/` — ESP-NOW peer mesh per the wire protocol in
  [`V6-ARCHITECTURE.md`](V6-ARCHITECTURE.md#wire-protocol).
  Channel-follow-STA, encrypted ESP-NOW (PMK/LMK), 3-second-button-
  hold pairing window, active heartbeat, 10 s health timeout.
- `components/topology/` — explicit topology config in NVS (straight /
  L / U / asymmetric custom) with per-segment device + LED-range
  assignments. Replaces v5's implicit-from-count model that broke
  asymmetric stairs.
- All 4 sensor priority modes preserved as fusion algorithms (MOST_RECENT,
  SLAVE_FIRST, MASTER_FIRST, ZONE_BASED) — but reframed as peer-fusion
  policies rather than master-decides logic.
- Coordinator election (lowest-MAC peer serves the web UI; mDNS and
  captive portal point at coordinator's IP).
- Config gossip with version-stamped `CONFIG_GOSSIP` packets; lex
  tiebreaker on MAC for simultaneous edits.

**Done criterion**: 2 devices on a U-stair, kill any one — the others
keep tracking; configure asymmetric segments and verify visually;
coordinator survives; rebooting the coordinator triggers a new election
within 5 s.

**Tag on completion**: `v6.0.0-alpha.3`.

---

## PR #5 — Real UI, MQTT, polish

**Status: not started.** Design source already in repo at
`frontend/design-source/` (Claude Design handoff: tokens.css + 7
screen JSXs).

Scope:
- `frontend/` — Vite + Preact + Tailwind + TypeScript scaffold. Lift
  the design tokens from `frontend/design-source/project/tokens.css`
  into `tailwind.config.ts`. Port each of the 7 screens from JSX into
  Preact components. Bundle target <80 KB gzipped.
- WebSocket live data wired to `/api/live` (replace polling-based
  placeholder from PR #2).
- OTA UI: drag-drop `.bin`, progress bar, 30 s reboot overlay polling
  `/api/version` to know when to dismiss.
- Pin remap UI with guard rails — unsafe pins shown disabled with
  tooltip, "reboot to apply" affordance.
- LD2450 2-D zone editor (only visible when `radar_kind=ld2450`) —
  drag-define exclusion zones in x/y space.
- Factory reset flow with type-device-name confirmation.
- MQTT publisher: off by default, system-tab toggle, Home Assistant
  auto-discovery payload format. Publishes `{distance, mode, state}`,
  subscribes to `cmd/*`.
- README rewrite for v6, hardware photos, install guide, HA integration
  re-validation against v6 endpoints.

**Done criterion**: full UI works on phone + desktop; all flows from
the design covered; MQTT integration tested with a real Home Assistant;
clean release notes.

**Tag on completion**: `v6.0.0`. Promote `v6-idf-rewrite` → `main`.
Move v5 README content fully behind a "v5 legacy" link.

---

## Tag/release cadence

| Tag                | Trigger        | Contents              |
| ------------------ | -------------- | --------------------- |
| `v5.1.1`           | (already exists) | last Arduino release |
| `v6.0.0-alpha.0`   | PR #1 merge    | skeleton boots only   |
| `v6.0.0-alpha.1`   | PR #2 merge    | + Wi-Fi, web, OTA     |
| `v6.0.0-alpha.2`   | PR #3 merge    | + radar + LED engine  |
| `v6.0.0-alpha.3`   | PR #4 merge    | + peer mesh           |
| `v6.0.0`           | PR #5 merge    | feature complete      |

CI auto-attaches per-board firmware artifacts to tag pushes.

---

## v6.x roadmap (post-6.0.0)

### v6.x architectural pivot (in progress, 2026-05-07)

Dropped the dual-device ESP-NOW master/slave architecture and locked
the firmware to one ESP32-C3 + one LD2450 per install. Removed the
`mesh` and `topology` components, the LD2410-family radar drivers, the
Mesh screen in the web UI, all `/api/mesh*` and `/api/topology*`
endpoints, and the `mesh` and `topo` NVS namespaces. See
[`V6-ARCHITECTURE.md`](V6-ARCHITECTURE.md) for the full justification
and what was kept.

### PCB rev (next)
- **Custom PCB based on ESP32-C3 + LD2450** — discrete AP2112K-3.3 or
  MIC5219 LDO feeding both the C3's 3.3 V rail and the LD2450 (avoids
  the SuperMini onboard LDO marginal under continuous radar load).
  Star-grounded LED-strip return to PSU; unpopulated I²C header for
  future env sensor or optional second LD2450.
- **Bench validation of single-sensor coverage** on real L-shape and
  U-shape benches at the inside-corner mounting position. Document
  range degradation at far-end of long arms.

### LED engine work
- **2-D zone mapping from LD2450 (x, y)** — currently the LED engine
  uses distance-only mapping. With single-sensor architecture locked,
  add a runtime "strip shape" config (straight / L / U) so target
  (x, y) projects to the correct LED segment. This replaces what the
  v6.0 topology component did across multiple devices.
- **Calibration UI** — drag-define the LED strip's 2-D path in the
  sensor's coordinate frame; persist to NVS.

### Hardening
- **Signed OTA** — `esp_secure_boot` + signed `.bin` with rollback. The
  bootloader rollback path is already armed; signing keys + UI
  workflow are pending.
- **Persistent sessions** — auth tokens currently live in RAM, lost on
  reboot. Move to NVS with explicit revocation list.

### UX / scale
- **LED count beyond 300** — verified; lift the documented soft cap to
  the theoretical 1500-pixel limit per device.
- **Realistic `sim` radar** — replay captured LD2450 frame logs.
- **Factory reset preserves Wi-Fi** option.

### Removed from this list (no longer relevant under single-sensor)
- ~~Encrypted ESP-NOW~~
- ~~Auto-topology learning across multi-device installs~~
- ~~Dual-core pinning to handle peer mesh load~~ (single-sensor C3 has
  enough headroom)
- ~~OTA preserves paired peers~~
