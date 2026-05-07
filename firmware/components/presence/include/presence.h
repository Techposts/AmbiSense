#pragma once

/*
 * AmbiSense v6.2 — presence detection.
 *
 * Derived output. Reads radar via radar_peek() at 10 Hz, applies a
 * configurable vacancy timeout, and publishes a debounced presence
 * snapshot to webui + future MQTT publisher.
 *
 * Why a separate component (vs folding into motion):
 *   motion's job is to produce a smooth single-target position for the
 *   LED renderer. presence's job is to answer "is anyone in the room
 *   right now, and how many?" — a different question with a different
 *   smoothing model (vacancy timer instead of Kalman). Splitting the
 *   responsibilities lets us swap radar driver underneath without
 *   touching either consumer.
 *
 * Sensor matrix (target_count semantics):
 *   ld2450  : 0..3 simultaneous targets, with x/y/v per target.
 *   ld2410c : 0 or 1 (binary detect + distance + state moving/stationary).
 *   ld2410  : same as 2410c.
 *   sim     : driven by sim trace.
 */

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool      occupied;          /* true if any target seen within vacancy_secs */
    uint8_t   target_count;      /* current number of targets (0..3) */
    bool      stationary;        /* true if at least one target has |v| < 5 cm/s
                                    (LD2410-family always reports this true when
                                    the radar's internal state is "stationary") */
    int16_t   nearest_cm;        /* distance to nearest target; -1 if vacant */
    uint32_t  last_seen_ms;      /* monotonic ms since boot of last detection;
                                    0 if never seen */
    uint32_t  ms_since_seen;     /* derived helper: now - last_seen_ms;
                                    UINT32_MAX if never seen */
    uint32_t  vacancy_secs;      /* configured timeout */
} presence_t;

esp_err_t presence_init(void);

/* Snapshot the latest presence state. Returns ESP_OK and fills *out. */
esp_err_t presence_get(presence_t *out);

/* Configure the vacancy timeout in seconds. Persists to NVS namespace
 * `presence` key `vacancy`. Default 60. Bounded [5, 3600]. */
esp_err_t presence_set_vacancy_timeout(uint32_t secs);
uint32_t  presence_get_vacancy_timeout(void);

#ifdef __cplusplus
}
#endif
