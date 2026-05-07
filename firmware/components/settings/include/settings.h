#pragma once

/*
 * AmbiSense v6 — settings facade over NVS.
 *
 * Replaces the v5 EEPROM byte-layout (config.h:150-217) with one NVS namespace
 * per concern.  Each module owns its namespace; this header only exposes the
 * cross-cutting init + the board namespace (used by main during boot).
 *
 * Namespaces (one per concern):
 *   sys    — device_name
 *   board  — board.id, pin overrides, radar_kind
 *   led    — count, brightness, rgb, mode, span, ...
 *   dist   — min_cm, max_cm
 *   motion — smoother mode + Kalman/PI gains
 *   wifi   — ssid, password, hostname, static ip
 *   auth   — pbkdf2 hash of admin password
 *
 * Removed in v6.x single-sensor rewrite: `mesh` and `topo` namespaces
 * (multi-device pairing + topology graph). Old keys may still exist in
 * NVS on units upgraded from v6.0; they are simply never read.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize NVS partition.  Erases and re-initializes if the partition
 * is corrupted or its layout has changed (handled internally by IDF). */
esp_err_t settings_init(void);

/* ---- board namespace ----------------------------------------------------
 * Stores: board.id (string), board.led_pin (u8), board.radar_rx (u8),
 *         board.radar_tx (u8), board.button (u8), board.status (u8),
 *         board.radar_kind (string: "ld2450" | "sim")
 *
 * On first boot every key is missing; callers fall back to the board
 * profile defaults (see components/board).
 */

/* Read the saved board id; copies up to `max` bytes into `out` (NUL-terminated).
 * Returns ESP_ERR_NVS_NOT_FOUND if no value has ever been written. */
esp_err_t settings_get_board_id(char *out, size_t max);
esp_err_t settings_set_board_id(const char *id);

/* Pin overrides — a value of BOARD_PIN_NONE means "use board profile default". */
esp_err_t settings_get_pin_override(const char *key, uint8_t *out);
esp_err_t settings_set_pin_override(const char *key, uint8_t pin);

esp_err_t settings_get_radar_kind(char *out, size_t max);
esp_err_t settings_set_radar_kind(const char *kind);

/* ---- wifi namespace -----------------------------------------------------
 * Stores: wifi.ssid (string), wifi.pass (string), wifi.hostname (string).
 * Note: v6.0 stores creds plaintext in NVS. v6.x will move to chip-key AES.
 */
esp_err_t settings_get_wifi_ssid(char *out, size_t max);
esp_err_t settings_set_wifi_ssid(const char *ssid);
esp_err_t settings_get_wifi_pass(char *out, size_t max);
esp_err_t settings_set_wifi_pass(const char *pass);
esp_err_t settings_get_hostname(char *out, size_t max);
esp_err_t settings_set_hostname(const char *name);

/* ---- sys namespace ------------------------------------------------------
 * Stores: sys.device_name (string).
 */
esp_err_t settings_get_device_name(char *out, size_t max);
esp_err_t settings_set_device_name(const char *name);

/* ---- auth namespace -----------------------------------------------------
 * Stores: auth.pw_hash (32-byte PBKDF2-SHA256 hash), auth.pw_salt (16 bytes).
 * Empty/missing means auth disabled.
 */
esp_err_t settings_get_auth_hash(uint8_t out_hash[32], uint8_t out_salt[16]);
esp_err_t settings_set_auth_hash(const uint8_t hash[32], const uint8_t salt[16]);
esp_err_t settings_clear_auth(void);

/* ---- led namespace ------------------------------------------------------
 * All LED-engine settings as typed accessors. PR #3 owns these.
 */
esp_err_t settings_get_u32(const char *ns, const char *key, uint32_t *out);
esp_err_t settings_set_u32(const char *ns, const char *key, uint32_t v);
esp_err_t settings_get_i32(const char *ns, const char *key, int32_t *out);
esp_err_t settings_set_i32(const char *ns, const char *key, int32_t v);
esp_err_t settings_get_u8(const char *ns, const char *key, uint8_t *out);
esp_err_t settings_set_u8(const char *ns, const char *key, uint8_t v);
esp_err_t settings_get_blob(const char *ns, const char *key, void *out, size_t *len);
esp_err_t settings_set_blob(const char *ns, const char *key, const void *data, size_t len);
esp_err_t settings_get_str(const char *ns, const char *key, char *out, size_t max);
esp_err_t settings_set_str(const char *ns, const char *key, const char *val);

#ifdef __cplusplus
}
#endif
