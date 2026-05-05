#pragma once

/*
 * AmbiSense v6 — embedded web server.
 *
 * One esp_http_server instance with all /api/... routes plus root + captive-
 * portal redirect endpoints (so iOS/Android pop the setup page in AP
 * mode). PR #5 replaces the inline placeholder HTML with the full Preact
 * UI served from LittleFS.
 *
 * Stub state hooks let PR #3/#4 push live data (distance, RSSI, mesh
 * health) without webui having to depend on radar/mesh components.
 */

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t webui_init(void);

/* Live state hooks. PR #2 publishes Wi-Fi telemetry; PR #3 publishes
 * distance + radar; PR #4 publishes peer health.
 * Updates are coalesced and emitted to all connected /api/live WS
 * clients at ~5 Hz. */
typedef struct {
    int16_t distance_cm;
    int8_t  direction;        /* -1, 0, +1 */
    int8_t  rssi;
    uint32_t free_heap;
    uint32_t uptime_s;
    uint8_t peer_count;
    uint8_t peer_healthy;
} webui_live_t;

void webui_publish_live(const webui_live_t *snap);

#ifdef __cplusplus
}
#endif
