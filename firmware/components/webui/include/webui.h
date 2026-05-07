#pragma once

/*
 * AmbiSense v6 — embedded web server.
 *
 * One esp_http_server instance with all /api/... routes plus root + captive-
 * portal redirect endpoints (so iOS/Android pop the setup page in AP
 * mode).
 *
 * Single-device architecture: every endpoint is local; there is no peer /
 * mesh / pairing API surface. Live data publishes the device's own radar
 * snapshot at 20 Hz over /api/live WS.
 */

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t webui_init(void);

/* Live state hooks. Updates are coalesced and emitted to all connected
 * /api/live WS clients at 20 Hz. */
typedef struct {
    int16_t distance_cm;       /* smoothed + predicted */
    int16_t raw_cm;            /* median-filtered but un-smoothed; lets the
                                * Motion screen draw raw vs smoothed without
                                * a second NVS round-trip */
    int8_t  direction;         /* -1, 0, +1 */
    int8_t  rssi;
    uint32_t free_heap;
    uint32_t uptime_s;
} webui_live_t;

void webui_publish_live(const webui_live_t *snap);

#ifdef __cplusplus
}
#endif
