# SmartGhar device protocol (v1.0)

This document is the **wire contract** every Techposts IoT device implements to be discovered and operated by the [smartghar Home Assistant integration](https://github.com/Techposts/smartghar-homeassistant). It also describes the integration-side dispatch model for adding a new product.

If you're building a new Techposts device (RidgeSync, future products), this is the doc to design against.

---

## Topology models

The protocol supports two device topologies. Both speak the same wire contract — the difference is whether `/api/v1/devices` returns one entry or many.

### A. Standalone hub (mains-powered, always-on)

Single ESP32 doing everything. The device IS its own hub; it presents a single virtual sub-device representing its own function.

Examples: **AmbiSense** (radar + LED, mains via USB or 5 V PSU), **mains-powered RidgeSync** (door lock with permanent power).

```
[AmbiSense ESP32-C3]
   ↑   mDNS _smartghar._tcp + REST + WS
   ↓
[Home Assistant + smartghar integration]
```

### B. Hub + TX (battery sensors via gateway)

Always-on hub ESP32 acts as a gateway for battery-powered TX nodes that talk to it over short-range RF (ESP-NOW, LoRa, BLE). Each TX node appears as a sub-device in the hub's `/api/v1/devices` array.

Examples: **TankSync** (one mains hub, multiple battery-powered tank sensors), **future battery-powered RidgeSync** if shipped as a multi-door kit.

```
[battery TX node 1]   [battery TX node 2]   [battery TX node N]
        ↘                    ↓                    ↙
                  RF link (ESP-NOW / LoRa)
                            ↓
                  [TankSync hub ESP32]
                            ↑   mDNS _smartghar._tcp + REST + WS
                            ↓
              [Home Assistant + smartghar integration]
```

### Choosing a model

| Situation | Use |
|---|---|
| Mains-powered, always-on, single sensor/actuator | Standalone hub |
| Battery-powered, infrequent reports, multi-sensor kit | Hub + TX |
| Mains-powered, but conceptually a fleet (e.g. multi-zone irrigation) | Hub + TX |
| Doesn't fit either | Open an issue before designing — likely we extend the protocol |

---

## Discovery — mDNS

Every device advertises:

```
Service type: _smartghar._tcp.local.
Port:         80 (HTTP) or 443 (HTTPS, future)
Hostname:     <product>-XXXX.local  (XXXX = last 4 hex chars of MAC)
TXT records:
  hub_id        <product>_<12-hex MAC>      (REQUIRED — stable identity)
  product       <product>                   (e.g. "ambisense", "tanksync", "ridgesync")
  manufacturer  SmartGhar                   (constant string, identifies the fleet)
  schema        1.0                         (protocol version this device speaks)
  path          /api/v1/info                (endpoint integration GETs first)
```

The integration's `config_flow` listens for `_smartghar._tcp` and reads `hub_id` to deduplicate / persist identity across HA reloads. Without `hub_id` the discovery is rejected.

### Implementation (ESP-IDF)

```c
mdns_init();
mdns_hostname_set("ambisense-aabb");
mdns_instance_name_set("AmbiSense");

mdns_service_add("_smartghar", "_smartghar", "_tcp", 80, NULL, 0);

mdns_txt_item_t txt[] = {
    { "hub_id",       "ambisense_d83bda3506f0" },
    { "product",      "ambisense"              },
    { "manufacturer", "SmartGhar"              },
    { "schema",       "1.0"                    },
    { "path",         "/api/v1/info"           },
};
mdns_service_txt_set("_smartghar", "_tcp", txt, 5);
```

You may **also** advertise a product-specific service (e.g. `_ambisense._http._tcp`) for backwards compatibility with older tools, but `_smartghar._tcp` with the TXT records above is what the integration looks for.

---

## REST contract — `/api/v1/*`

### `GET /api/v1/info`

Hub identity + diagnostics. Polled every 30 s by the integration.

```json
{
  "schema_version": "1.0",
  "manufacturer":   "SmartGhar",
  "product":        "ambisense",
  "model":          "AmbiSense v6",
  "fw_version":     "v6.2.0-alpha.2",
  "hub_id":         "ambisense_d83bda3506f0",
  "unique_id":      "ambisense_d83bda3506f0",
  "host":           "ambisense-06f0",
  "ip":             "192.168.1.42",
  "uptime_s":       12345,
  "wifi_rssi":      -45,
  "free_heap":      60384,
  "capabilities":   ["presence", "distance", "led_strip"]
}
```

Required keys: `schema_version`, `manufacturer`, `product`, `model`, `fw_version`, `hub_id`. Everything else is best-effort but recommended.

### `GET /api/v1/devices`

Sub-device list. Each entry is a logical sensor/actuator the hub manages.

```json
{
  "devices": [
    {
      "kind":   "presence",
      "id":     0,
      "name":   "Presence Sensor",
      "state":  { "occupied": true, "target_count": 1, "nearest_cm": 80, ... },
      "config": { "vacancy_secs": 60, "radar_kind": "ld2450" }
    }
  ]
}
```

Key conventions:

| Key | Type | Notes |
|---|---|---|
| `kind` | string | one of `tank`, `power`, `pump_relay`, `gas`, `soil`, `door`, `air`, `presence`, `lock` (extend as new products land) |
| `id` | integer | stable per device within this hub. For standalone hubs always `0`. For hub+TX, indexed (often by RF address). |
| `name` | string | human-readable, shown in HA |
| `state` | object | live measurements. Schema is **kind-specific**. Common keys: `rssi_dbm`, `conn_state` ("online" / "offline" / "unreachable") |
| `config` | object | knobs the integration can update via PUT. Kind-specific. |

### `PUT /api/v1/devices/{id}`

Update config. Body shape:

```json
{ "config": { "vacancy_secs": 30 } }
```

Returns updated device object so the integration can reflect changes immediately:

```json
{ "ok": true, "updated": 1, "device": { ...full device object... } }
```

Auth-gated by the device's admin password (cookie session from `/api/auth/login`).

### `POST /api/v1/hub/identify`

Flash the device's status LED for ~3 seconds at 5 Hz so a user can physically locate which device they're operating in HA.

```json
{ "ok": true }
```

### `POST /api/v1/hub/reboot`

Soft reboot. Auth-gated.

### `POST /api/v1/hub/ota/check` and `/install`

Optional. Reserved for future OTA-via-integration flow. AmbiSense v6.2 returns 501 Not Implemented — OTA is currently web-UI-only.

---

## WebSocket — `/api/v1/stream`

Real-time push. Integration opens one WS per discovered device after `/api/v1/info` succeeds.

### Cadence
- Snapshot frame every **~3 seconds** containing the latest devices array
- Heartbeat every **20 seconds** (empty `{}` or `{"hb": true}`) to keep the connection warm
- Reconnect with exponential backoff (2 s → 60 s) if dropped

### Frame shape

```json
{
  "type":    "snapshot",
  "ts":      1234567890,
  "info":    { ...same shape as /api/v1/info... },
  "devices": [ ...same shape as /api/v1/devices... ]
}
```

The integration uses this for live entity updates without burning HTTP bandwidth.

### Implementation note

If you don't have time to implement a separate WS endpoint, the integration silently falls back to HTTP polling on `/api/v1/info` + `/api/v1/devices` every 30 s. This is acceptable for telemetry that doesn't need sub-second updates (e.g. tank levels). It's NOT acceptable for occupancy/door events (5 s+ stale state would cause user-visible automation lag).

For new mains-powered products: implement the WS. For battery-powered products that wake briefly to send a packet: skip the WS, lean on the hub's WS for any sub-device the hub knows about.

---

## Auth model

Every device that accepts writes (PUT, POST) MUST gate them with the existing admin-password cookie session model:

1. Device exposes `POST /api/auth/login` accepting `{password: "..."}` and returning a `Set-Cookie: session=...` header
2. The integration logs in once at setup, persists the cookie in HA's credential store
3. All subsequent writes include the cookie
4. Read-only endpoints (`GET /api/v1/info`, `GET /api/v1/devices`, WS `/api/v1/stream`) are unauthenticated for LAN-local discovery to work

For high-stakes products (RidgeSync — fingerprint locks), consider stronger auth:
- Per-command nonces (challenge-response from device, signed reply from integration)
- API tokens with finite scope (e.g. `unlock-only` token vs `admin` token)
- TLS via `mqtts://` or HTTPS with a chip-derived self-signed cert

For low-stakes products (presence, tank levels), the cookie session is sufficient.

---

## Schema versioning

`schema_version` in `/api/v1/info` is the **public protocol version**. When we evolve the protocol:

- **Patch (1.0 → 1.1)**: backwards-compatible additions only (new optional fields, new device kinds, new endpoints). Old integration versions ignore unknown keys; new integration versions detect via `capabilities` whether an optional feature is supported.
- **Minor (1.x → 2.0)**: breaking change. Coordinated firmware + integration release. We try hard to never need this. If we do, the integration supports both 1.x and 2.0 in parallel for one major version cycle.

**Forward compat rule**: every new device kind MUST be ignorable by older integration versions. If a TankSync user updates AmbiSense firmware to v6.3 (which adds `kind: "led_strip"` as a separate device) but hasn't updated the smartghar integration, the integration silently drops the unknown kind and continues to handle the rest. No errors, no broken state.

---

## Adding a new device kind to the smartghar integration

When you ship a new product (e.g. RidgeSync), add to `smartghar-homeassistant/custom_components/smartghar/`:

### 1. `const.py` — register the kind

```python
DEVICE_KIND_LOCK = "lock"
DEVICE_KIND_PRESENCE = "presence"
DEVICE_KINDS = [
    # ... existing ...
    DEVICE_KIND_LOCK,
    DEVICE_KIND_PRESENCE,
]
```

### 2. Entity builders

For sensors that read from `state` keys, extend the relevant entity file:

```python
# binary_sensor.py
class PresenceOccupancyEntity(SmartGharEntity, BinarySensorEntity):
    _attr_device_class = BinarySensorDeviceClass.OCCUPANCY
    @property
    def is_on(self):
        return self.device["state"].get("occupied", False)

# sensor.py
class PresenceDistanceEntity(SmartGharEntity, SensorEntity):
    _attr_device_class = SensorDeviceClass.DISTANCE
    _attr_native_unit_of_measurement = "cm"
    @property
    def native_value(self):
        return self.device["state"].get("nearest_cm")
```

### 3. Dispatch on kind

In `__init__.py` or coordinator, when iterating `coordinator.devices`, dispatch entity construction:

```python
def build_entities_for_device(device, coordinator):
    kind = device["kind"]
    if kind == DEVICE_KIND_TANK:
        return [TankLevelEntity(...), TankVoltageEntity(...)]
    if kind == DEVICE_KIND_PRESENCE:
        return [PresenceOccupancyEntity(...), PresenceDistanceEntity(...)]
    if kind == DEVICE_KIND_LOCK:
        return [LockEntity(...), DoorContactEntity(...)]
    _LOGGER.warning("Unknown device kind %s, skipping", kind)
    return []
```

### 4. Bump integration version + changelog

Update `manifest.json` `version` field, add a CHANGELOG entry referencing the new kind and the firmware version that introduced it.

---

## Templates per device kind

These are starter schemas — a new product author copies one and customizes the `state` / `config` keys.

### `presence` (AmbiSense)

```json
{
  "kind":   "presence",
  "id":     0,
  "name":   "Presence Sensor",
  "state":  {
    "occupied":           true,
    "target_count":       1,
    "stationary":         false,
    "nearest_cm":         80,
    "seconds_since_seen": 0,
    "rssi_dbm":           -45,
    "conn_state":         "online"
  },
  "config": {
    "vacancy_secs": 60,
    "radar_kind":   "ld2450"
  }
}
```

### `tank` (TankSync — already in production)

```json
{
  "kind":   "tank",
  "id":     1,
  "name":   "Overhead Tank",
  "state":  {
    "level_pct":   72,
    "voltage":     3.84,
    "rssi_dbm":    -68,
    "conn_state":  "online",
    "last_seen_s": 12
  },
  "config": {
    "capacity_l":   1000,
    "alert_low":    20,
    "alert_high":   90
  }
}
```

### `lock` (RidgeSync — proposed)

```json
{
  "kind":   "lock",
  "id":     0,
  "name":   "Front Door",
  "state":  {
    "locked":         true,
    "door_open":      false,
    "battery_pct":    87,
    "tamper":         false,
    "last_unlock":    { "by": "fingerprint:#3", "ts": 1234567890 },
    "rssi_dbm":       -52,
    "conn_state":     "online"
  },
  "config": {
    "auto_lock_secs": 30,
    "tamper_alarm":   true
  }
}
```

Commands like `unlock`, `add_fingerprint` would arrive via `PUT /api/v1/devices/{id}` body like:

```json
{ "command": "unlock", "args": { "by": "ha_user_admin" } }
```

---

## Testing checklist for a new product

Before claiming smartghar-compat:

- [ ] mDNS service `_smartghar._tcp` advertises with all 5 required TXT records
- [ ] `GET /api/v1/info` returns 200 with all required keys + plausible values
- [ ] `GET /api/v1/devices` returns a non-empty array with the right `kind` per sub-device
- [ ] `PUT /api/v1/devices/{id}` round-trips a config change (verify via subsequent GET)
- [ ] `POST /api/v1/hub/identify` makes the LED visibly flash
- [ ] `POST /api/v1/hub/reboot` causes a clean reboot
- [ ] WS `/api/v1/stream` emits at least one snapshot frame within 5 s of connect, then heartbeats every 20 s
- [ ] Auth-gated endpoints reject unauthenticated writes with 401
- [ ] Pulling the power and bringing the device back: integration reconnects + entities go available
- [ ] Add the kind to `smartghar-homeassistant/custom_components/smartghar/const.py` + entity builders
- [ ] Bench test: install the updated integration in HA, watch the device auto-discover, confirm entities populate

---

## Quick reference: what's in AmbiSense's contract

| What | Where |
|---|---|
| mDNS service registration | [`netmgr.c:bring_up_mdns()`](../firmware/components/netmgr/netmgr.c) |
| `/api/v1/info` handler | [`webui.c:handle_v1_info()`](../firmware/components/webui/webui.c) |
| `/api/v1/devices` handler | [`webui.c:handle_v1_devices()`](../firmware/components/webui/webui.c) |
| `build_presence_device()` device builder | [`webui.c`](../firmware/components/webui/webui.c) |
| `/api/v1/devices/0` PUT handler | [`webui.c:handle_v1_devices_put()`](../firmware/components/webui/webui.c) |
| `/api/v1/hub/identify` | [`webui.c:handle_v1_hub_identify()`](../firmware/components/webui/webui.c) |
| `/api/v1/hub/reboot` | [`webui.c:handle_v1_hub_reboot()`](../firmware/components/webui/webui.c) |

WS `/api/v1/stream` is **not yet implemented** in v6.2.0-alpha.2 — falls back to polling per the spec. Will land in v6.2.0-alpha.3 along with the integration-side `kind: "presence"` PR.
