#pragma once

/*
 * AmbiSense v6 — motion smoother (port of v5 radar_manager.cpp:38-198).
 *
 * Low-pass distance + velocity estimator + PI controller, runs in its own
 * task at ~50 Hz. Consumes radar_frame_t, publishes a smoothed target_t.
 *
 * Tunables come from NVS namespace `motion` (set via /api/settings).
 * Defaults match v5: position_smooth=0.2, velocity_smooth=0.1,
 * predict=0.5, p_gain=0.1, i_gain=0.01.
 */

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool      present;
    int16_t   distance_cm;     /* smoothed + predicted */
    int16_t   raw_cm;          /* spike-rejected (median) but un-smoothed —
                                * for the Motion screen's raw vs smoothed
                                * line chart so users can see the smoother
                                * actually working. */
    int8_t    direction;
    uint8_t   energy;
    uint64_t  ts_us;
} target_t;

esp_err_t motion_init(void);

/* Get the latest smoothed target. Non-blocking. */
void motion_get(target_t *out);

#ifdef __cplusplus
}
#endif
