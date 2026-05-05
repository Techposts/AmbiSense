#pragma once

/*
 * AmbiSense v6 — network manager.
 *
 * Owns the Wi-Fi state machine: tries STA with stored credentials, falls
 * back to AP "AmbiSense-XXXX" if STA fails or no credentials are saved.
 * Brings up mDNS (`<name>.local`), and runs a captive-portal DNS responder
 * while in AP mode so phones auto-pop the setup page.
 *
 * State changes are exposed via callback so the status_led component can
 * mirror them as blink patterns.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NETMGR_STATE_BOOT = 0,
    NETMGR_STATE_STA_CONNECTING,
    NETMGR_STATE_STA_CONNECTED,
    NETMGR_STATE_AP_FALLBACK,    /* AP because STA failed or no creds */
    NETMGR_STATE_OTA,
    NETMGR_STATE_ERROR,
} netmgr_state_t;

typedef void (*netmgr_state_cb_t)(netmgr_state_t state, void *ctx);

/* Bring up the Wi-Fi stack, mDNS, and captive portal as needed. Reads
 * stored credentials from NVS (`wifi.ssid` / `wifi.pass`).  Non-blocking;
 * connection happens on the IDF event loop. */
esp_err_t netmgr_init(void);

/* Connect with new credentials and persist them on success. Pass NULL ssid
 * to clear creds and force AP fallback. */
esp_err_t netmgr_set_credentials(const char *ssid, const char *pass);

/* Currently in STA mode? */
bool netmgr_is_sta_connected(void);

/* IP/hostname accessors — caller-provided buffers. */
esp_err_t netmgr_get_ip(char *out, size_t max);
esp_err_t netmgr_get_hostname(char *out, size_t max);

/* RSSI in STA mode, 0 in AP mode. */
int8_t netmgr_get_rssi(void);

/* Register a callback fired on every state transition. */
void netmgr_on_state_change(netmgr_state_cb_t cb, void *ctx);

/* Set device hostname (lower-case, alnum/hyphen). Persists to NVS. */
esp_err_t netmgr_set_hostname(const char *name);

#ifdef __cplusplus
}
#endif
