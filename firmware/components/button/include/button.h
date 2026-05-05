#pragma once

/*
 * AmbiSense v6 — physical button driver.
 *
 * Single-button polling driver designed for the C3 SuperMini's BOOT
 * button (GPIO 9, active-low, internal pull-up). Drives three callbacks:
 *
 *   short  — press < 1 s. Could be used for "toggle LEDs" later; unused
 *            in v6.0.
 *   long   — press >= 3 s. Wired to mesh_open_pairing(): the standard
 *            "physically pair this device" gesture.
 *   verylong — press >= 10 s. Wired to factory reset (TODO; not in v6.0).
 *
 * Polled at 50 Hz from a tiny dedicated task. Polling beats GPIO ISRs for
 * mechanical buttons because the state machine inherently debounces — a
 * spurious 1 ms blip simply doesn't survive across two 20 ms samples.
 */

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BUTTON_PRESS_SHORT     = 1,
    BUTTON_PRESS_LONG      = 2,    /* >= 3 s */
    BUTTON_PRESS_VERYLONG  = 3,    /* >= 10 s */
} button_event_t;

typedef void (*button_event_cb_t)(button_event_t evt);

/* Initialize and start the polling task. `gpio_num` is the button pin;
 * `active_low` true means the button reads 0 when pressed (default for
 * BOOT-style pull-up wiring). */
esp_err_t button_init(uint8_t gpio_num, bool active_low, button_event_cb_t cb);

#ifdef __cplusplus
}
#endif
