#pragma once

/*
 * AmbiSense v6 — onboard status LED driver.
 *
 * Encodes system state as blink patterns on the board's onboard LED. A
 * dedicated FreeRTOS task owns the GPIO; any subsystem can change pattern
 * without coordinating with the LED loop.
 *
 * Patterns:
 *   OFF        — fully dark.
 *   BOOT       — solid on while bringing up subsystems.
 *   AP_MODE    — slow blink (1 Hz) while waiting for Wi-Fi config.
 *   STA_MODE   — heartbeat (two short pulses then pause) when connected.
 *   OTA        — fast blink (5 Hz) during firmware update.
 *   ERROR      — long-short-long pattern (recoverable error).
 *   PANIC      — SOS pattern (· · · — — — · · ·) before reset.
 */

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    STATUS_LED_OFF = 0,
    STATUS_LED_BOOT,
    STATUS_LED_AP_MODE,
    STATUS_LED_STA_MODE,
    STATUS_LED_OTA,
    STATUS_LED_ERROR,
    STATUS_LED_PANIC,
} status_led_pattern_t;

/* gpio_num: BCM-style GPIO number; active_low: true if onboard LED sinks
 * current (typical on C3 SuperMini onboard LED on GPIO 8). */
esp_err_t status_led_init(uint8_t gpio_num, bool active_low);

/* Switch pattern. Thread-safe; safe to call from any task. */
void status_led_set_pattern(status_led_pattern_t pattern);

/* Run `pattern` for `duration_ms`, then automatically revert to whatever
 * the last stable pattern was. Lets transient subsystems flash a status
 * code without having to track and restore prior state. */
void status_led_oneshot(status_led_pattern_t pattern, uint32_t duration_ms);

#ifdef __cplusplus
}
#endif
