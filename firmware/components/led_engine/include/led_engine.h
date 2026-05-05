#pragma once

/*
 * AmbiSense v6 — LED engine.
 *
 * Drives a WS2812(B) strip via the ESP-IDF managed-component `led_strip`
 * (RMT-backed, non-blocking refresh). Renders one of 11 visual modes
 * ported from v5 led_controller.cpp at 60 Hz, consuming the smoothed
 * target_t from components/motion.
 */

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Mode IDs match v5 (config.h:57-67) so any external integration that
 * spoke the v5 numeric mode keeps working. */
typedef enum {
    LED_MODE_STANDARD         = 0,
    LED_MODE_RAINBOW          = 1,
    LED_MODE_COLOR_WAVE       = 2,
    LED_MODE_BREATHING        = 3,
    LED_MODE_SOLID            = 4,
    LED_MODE_COMET            = 5,
    LED_MODE_PULSE            = 6,
    LED_MODE_FIRE             = 7,
    LED_MODE_THEATER_CHASE    = 8,
    LED_MODE_DUAL_SCAN        = 9,
    LED_MODE_MOTION_PARTICLES = 10,
} led_mode_t;

esp_err_t led_engine_init(uint8_t data_gpio);

/* Force a parameter reload from NVS (called when /api/settings POST changes
 * any LED key). */
void led_engine_reload(void);

#ifdef __cplusplus
}
#endif
