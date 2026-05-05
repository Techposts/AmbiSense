#pragma once

/*
 * AmbiSense v6 — settings facade over NVS.
 *
 * Replaces the v5 EEPROM byte-layout (config.h:150-217) with one NVS namespace
 * per concern.  Each module owns its namespace; this header only exposes the
 * cross-cutting init + the board namespace (used by main during boot).
 *
 * Namespaces (one per concern, populated across PRs):
 *   sys    — device_name, role-related keys
 *   board  — board.id, pin overrides, radar_kind        ← PR #1 (this file)
 *   led    — count, brightness, rgb, mode, span, ...    ← PR #3
 *   dist   — min_cm, max_cm                             ← PR #3
 *   motion — pi smoother gains                          ← PR #3
 *   mesh   — peers blob, channel, encryption keys       ← PR #4
 *   topo   — topology kind + segments blob              ← PR #4
 *   wifi   — ssid, encrypted password, static ip        ← PR #2
 *   auth   — pbkdf2 hash of admin password              ← PR #2
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
 *         board.radar_kind (string: "ld2410" | "ld2412" | "ld2420" |
 *                                   "ld2450" | "sim")
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

#ifdef __cplusplus
}
#endif
