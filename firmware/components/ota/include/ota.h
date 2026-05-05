#pragma once

/*
 * AmbiSense v6 — OTA firmware update.
 *
 * Streams a multipart/form-data firmware upload into the inactive OTA
 * partition. On valid hash, marks the partition for boot and reboots.
 * Bootloader rollback is enabled (sdkconfig.defaults), so a failed boot
 * automatically reverts to the previous slot.
 *
 * Unsigned for v6.0 (per architecture decision); signed-OTA on roadmap.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ota_session_s ota_session_t;

/* Begin a new OTA session. Returns a handle the caller streams data into. */
ota_session_t *ota_begin(size_t expected_size);

/* Append bytes to the in-progress session. Returns ESP_OK or an error;
 * on error the session is automatically aborted and freed. */
esp_err_t ota_write(ota_session_t *s, const void *data, size_t len);

/* Finish: validate, mark next-boot, schedule reboot. Frees the session. */
esp_err_t ota_finish(ota_session_t *s);

/* Abort and free without committing. Safe to call on any state. */
void ota_abort(ota_session_t *s);

/* Mark the running firmware valid (call once at boot if everything's OK,
 * defeats the rollback timer). */
esp_err_t ota_mark_valid(void);

#ifdef __cplusplus
}
#endif
