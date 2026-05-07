# Architectural decision log

The "why" behind v6.x. Each entry documents a decision that's load-bearing across the codebase, with the alternatives considered and the reason the chosen path won. Read top-to-bottom for the journey, or grep for a specific topic.

Format borrowed from [ADR](https://adr.github.io) (Architecture Decision Records) but kept lighter. Each entry has: **status** (Locked / Tentative / Reverted), **context**, **decision**, **alternatives**, **why this won**.

---

## D-001 — Single-sensor architecture (v6.x pivot)

- **Date:** 2026-05-07
- **Status:** Locked
- **Context:** v6.0.0 shipped a dual-device ESP-NOW master/slave architecture. Bench-tested on real L-shape and U-shape stair installs, surfaced two structural issues: (a) ESP-NOW round-trip jitter (5–100 ms with retries) added visible LED lag the motion Kalman filter couldn't hide, (b) the LD2450's `(x, y, speed)` target stream from one sensor at the inside-corner mounting position covers both arms of an L or U inside its 60° cone — making the second sensor redundant for the geometries users actually install.
- **Decision:** Drop dual-device entirely in v6.x. One ESP32-C3 + one LD2450 per install. Mount at the inside corner (L) or centre-back (U) with line-of-sight to both arms. Lock the v6.x kit PCB around this assumption.
- **Alternatives considered:**
  - **Keep dual-device + tune ESP-NOW timeouts harder.** Rejected — the radar latency budget (~50 ms total) is too tight for a wireless hop in the worst case; tuning timeouts down past a certain point increases retry frequency, not reduces it.
  - **Add encrypted ESP-NOW + auto-topology learning** (originally on the v6.x roadmap). Rejected — papering over the real problem (latency + install friction) with more complexity.
  - **Lean into single-sensor but keep LD2410 as default for cost.** Rejected — LD2410 is single-target distance-only; can't drive 2-D LED zone mapping. LD2450 (~$15) is the right kit default; LD2410C is a presence-only opt-in.
- **Why this won:** smaller BOM, faster (no wireless hop), simpler firmware (-1569 lines net), simpler install (no pairing flow), product positioning that scales to room-presence + HA integration. The trade-off accepted is documented mounting constraints for L/U installs — acceptable for a kit product.
- **See also:** [`V6-ARCHITECTURE.md`](V6-ARCHITECTURE.md) full architectural spec; [Wiki FAQ → "Why was the mesh dropped?"](https://github.com/Techposts/AmbiSense/wiki/FAQ#why-was-the-mesh--multi-device-support-dropped-in-v6x) user-facing version.

---

## D-002 — ESP32-C3 SuperMini as the kit MCU (recommendation flip)

- **Date:** 2026-05-07 (alongside D-001)
- **Status:** Locked
- **Context:** v6.0 recommended ESP32-S3 specifically because the dual-device peer mesh + per-peer fusion + ESP-NOW broadcast on a single C3 core ran hot under load. The "300 ms client-side debounced save" that shipped in v6.0 was a workaround for HTTP saturation under that combined load.
- **Decision:** Make the C3 SuperMini the kit reference. S3 demoted to "supported but overkill for a single-sensor install."
- **Alternatives considered:**
  - **Keep S3 as kit default.** Rejected — single-core C3 has plenty of headroom now that there's no peer broadcast and no second sensor to fuse. S3's dual-core advantage is wasted.
  - **C6 / C2.** Rejected — C6 has less SRAM than C3 + Wi-Fi 6 doesn't help; C2 is too constrained for the UI bundle.
- **Why this won:** smallest package, native USB-Serial-JTAG (no CH340 driver hassle), ~$3 BOM. With v6.x's reduced compute, plenty of headroom remains for v6.x roadmap features (presence detection, smartghar contract, future MQTT-as-opt-in if needed).
- **See also:** [Wiki Hardware Setup → "Board recommendation"](https://github.com/Techposts/AmbiSense/wiki/Hardware-Setup#board-recommendation-v6x).

---

## D-003 — LD2450 default + LD2410(C) restored as opt-in for presence-only

- **Date:** 2026-05-07 (v6.2.0-alpha.1)
- **Status:** Locked
- **Context:** v6.x originally stripped LD2410 / LD2412 / LD2420 drivers because LED follow-me needs (x, y) coordinates that LD2410 doesn't expose. Adding presence detection to v6.2 surfaced a real use case for static-presence-optimised sensors.
- **Decision:** Restore LD2410 driver. Expose `ld2450` (default), `ld2410c` (presence-only, recommended for static), `ld2410` (legacy compat), `sim` in the radar registry. Switch via Hardware tab → reboot.
- **Alternatives considered:**
  - **LD2450 only.** Rejected — LD2450 can lose track of fully-still targets after 30–60 s; for "couch reading" use cases, LD2410C's micro-motion detection is meaningfully better.
  - **Multiple sensors per device.** Rejected by D-001's single-sensor lock.
- **Why this won:** different sensors fit different jobs. Ship LD2450 for the kit (covers most use cases), let advanced users opt into LD2410C for static-presence-optimised installs. Same firmware, runtime selectable, no reflash needed.
- **See also:** [Wiki LD2450 vs LD2410C](https://github.com/Techposts/AmbiSense/wiki/Hardware-Setup#sensor-choice).

---

## D-004 — SmartGhar HA integration over MQTT publisher

- **Date:** 2026-05-07 (v6.2.0-alpha.2)
- **Status:** Locked (with re-evaluation door open)
- **Context:** v6.2.0-alpha.1 dev branch implemented an MQTT publisher with HA auto-discovery (~600 lines, +~150 KB partition). Discussed shipping it as an opt-in feature. While still unshipped, surfaced that the user already has a centralized HA custom integration repo (`smartghar-homeassistant`, supporting TankSync) — and wants new products (AmbiSense, RidgeSync) to feed into the same integration rather than each shipping its own MQTT auto-discovery.
- **Decision:** Drop MQTT from firmware. Implement the smartghar wire contract (`/api/v1/info` + `/api/v1/devices` + `/api/v1/devices/{id}` PUT + `/api/v1/hub/identify` + `/api/v1/hub/reboot` + WS `/api/v1/stream`) which the smartghar-homeassistant integration consumes via mDNS discovery + HTTP/WS.
- **Alternatives considered:**
  - **Ship MQTT as opt-in alongside smartghar contract.** Rejected for v6.2 — the partition cost (-9% free) and complexity (broker setup for users) didn't carry weight when the smartghar integration handles the use case faster (~5 ms LAN hop vs ~10–15 ms via broker) and simpler (no broker).
  - **HA-native auto-discovery (firmware publishes `homeassistant/...` configs).** Rejected — duplicate-entity risk for users running the smartghar integration; we already have a no-MQTT RESTful sensor recipe as the no-broker fallback.
- **Why this won:** real-time push without broker, HACS-installable in one click, scales across the Techposts product fleet (TankSync + AmbiSense + future), entity logic centralised in the integration repo (easier to update than firmware on N units in the field).
- **Door open for return:** if users running Node-RED / Grafana / non-HA automation systems surface real demand for MQTT, the firmware code is in git history — cherry-pick from around 2026-05-07 in the `feature/mqtt-optional` workspace, ship as opt-in.
- **See also:** [`SMARTGHAR-PROTOCOL.md`](SMARTGHAR-PROTOCOL.md) wire contract; [smartghar-homeassistant](https://github.com/Techposts/smartghar-homeassistant) integration repo; [Wiki Home Assistant Integration](https://github.com/Techposts/AmbiSense/wiki/Home-Assistant-Integration).

---

## D-005 — One HA integration for the entire Techposts product fleet

- **Date:** 2026-05-07 (alongside D-004)
- **Status:** Locked
- **Context:** Pre-existing per-product HA integrations: `ambisense-homeassistant` (described as "too old" by the user), `Aqualevel-HA-Integration` (older TankSync integration). Adding RidgeSync, PowerSync, and future products would have multiplied this fragmentation.
- **Decision:** One integration (`smartghar-homeassistant`) covers the entire fleet. Per-product code lives behind a `kind` dispatcher; adding a new product is an additive PR (new `DEVICE_KIND_*`, new entity classes, new translations, version bump).
- **Alternatives considered:**
  - **Per-product integrations.** Rejected — fragmentation, N integrations to maintain, N installs for a fleet user.
  - **Single firmware-level Tasmota-style metaprotocol.** Rejected — too much firmware complexity for the win; products have genuinely different state shapes (water level, presence, lock state) that benefit from explicit kind dispatch.
- **Why this won:** matches industry pattern (Shelly, ESPHome, Tuya — one integration, many products). Cross-product groupings ("all sensors in the kitchen") work natively. New products ship as small PRs.
- **See also:** [`project_smartghar_fleet.md` (memory)](../../.claude/projects/-Users-techposts-Projects-AmbiSense/memory/project_smartghar_fleet.md); [smartghar-homeassistant v0.7.0 release](https://github.com/Techposts/smartghar-homeassistant/releases/tag/v0.7.0).

---

## D-006 — Standalone-hub topology for AmbiSense; hub+TX for battery products

- **Date:** 2026-05-07 (smartghar protocol design)
- **Status:** Locked
- **Context:** TankSync uses a hub+TX architecture (always-on hub gateways for battery-powered sensor TX nodes via short-range RF). AmbiSense is a single mains-powered ESP32 doing everything. Two genuinely different topologies needed to fit one wire contract.
- **Decision:** Both topologies share the same `/api/v1/info` + `/api/v1/devices` schema. Standalone hub returns `/api/v1/devices` with one entry (the device itself as a virtual sub-device). Hub+TX returns N entries (one per TX node).
- **Alternatives considered:**
  - **Two protocols, dispatched by some flag.** Rejected — extra complexity, two integration code paths.
  - **Make AmbiSense pretend to be a "hub with no devices" that exposes presence at the hub level.** Rejected — breaks the "kind dispatch builds entities" pattern and forces special-cased hub-level entity logic.
- **Why this won:** uniform integration code; AmbiSense models itself as a hub with id=0 device of `kind: "presence"`, follows the same dispatch path TankSync's `kind: "tank"` does.
- **See also:** [`SMARTGHAR-PROTOCOL.md` § Topology models](SMARTGHAR-PROTOCOL.md#topology-models).

---

## D-007 — Always test before commit and release

- **Date:** 2026-05-07 (after the AP-teardown bug shipped in `v6.1.0-alpha.1`)
- **Status:** Locked discipline
- **Context:** v6.1.0-alpha.1 was committed + released as a public pre-release before bench-validating on real hardware. The AP-teardown bug (in-memory `sta_configured` flag never updated when credentials saved at runtime) wouldn't have surfaced in any build check — only on physical hardware doing the captive-portal onboarding. User had to file the issue post-release.
- **Decision:** Build-green is necessary but NOT sufficient. Before any `git commit`, `git push`, or `gh release create`: physically flash the firmware to a device and walk through the affected feature path. Confirm headline behaviour. Only then commit.
- **Alternatives considered:**
  - **Build-green + smoke test in simulator.** Rejected — `sim` driver doesn't exercise STA association, captive portal, or any interaction-driven feature.
  - **CI integration test on real hardware.** Aspirational — not yet set up. The user-driven manual bench test is the gate today.
- **Why this won:** caught what build checks can't. Releases are publicly visible and hard to retract; the asymmetry of "wait an hour to flash and verify" vs "ship a broken binary that strangers download" demands the test gate every time.
- **See also:** [`feedback_test_before_commit.md` (memory)](../../.claude/projects/-Users-techposts-Projects-AmbiSense/memory/feedback_test_before_commit.md).

---

## D-008 — Reverted: MQTT publisher in firmware (v6.2.0-alpha.1 dev branch only)

- **Date:** Implemented + reverted 2026-05-07
- **Status:** Reverted
- **Context:** Implemented MQTT publisher with HA auto-discovery as a candidate for v6.2.0-alpha.1. Built clean, never shipped a release. Cost ~150 KB partition (firmware went from 18% free to 9% free with `mqtt_client` + cJSON pulled in for retain/last-will/discovery payloads).
- **Decision:** Revert before shipping. Pivot to the smartghar HA integration model (D-004).
- **Why:** the smartghar HA custom integration handles the use case faster, simpler, and more uniformly across the product fleet. MQTT was the right answer when each product spoke its own protocol; once the fleet shares a contract, MQTT's fan-out advantage doesn't apply.
- **What survives:** `_smartghar._tcp` mDNS service registration is reused. The ground-truth state-publishing logic lives in the smartghar contract endpoints (`/api/v1/devices`) — same data, served over HTTP/WS instead of pushed to a broker.
- **Code preserved at:** `feature/mqtt-optional` development workspace (not pushed as a branch — exists in local stash + git reflog around 2026-05-07). Cherry-pickable if a future opt-in is justified.

---

## How to add a new entry

When a decision rises to "this affects how the codebase is organised" or "this constrains future product design," document it here:

1. Pick the next `D-NNN` number
2. Fill out: status, date, context, decision, alternatives, why this won
3. Link to relevant memory files / wiki pages / spec docs
4. If a future decision overrides this one, change the status to "Reverted" or "Superseded by D-NNN" — never delete the entry

Decisions that DON'T belong here:
- Bug fixes (those go in commit messages + CHANGELOG)
- Coding style tweaks (those go in a style guide)
- Day-to-day implementation choices (those go in code comments)

Decisions that DO belong here:
- Anything you'd want a successor maintainer to read on day one
- Anything where you imagine someone proposing the rejected alternative six months from now
