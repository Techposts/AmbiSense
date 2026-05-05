#pragma once

/*
 * AmbiSense v6 — board abstraction layer.
 *
 * Each supported ESP32 variant ships a static board_profile_t describing its
 * sane defaults and the pins that are unsafe to use as user-configurable I/O
 * (strapping pins, USB-JTAG pins, flash pins, etc.).  At runtime the active
 * profile is selected from NVS by id; pins on top can be individually
 * overridden by the user from the web UI, but writes to unsafe_pin_mask pins
 * are rejected before the override is persisted.
 *
 * Adding a board = +1 board_profile_t entry in board.c.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Sentinel for "no pin assigned". GPIO_NUM_NC (-1) cast to unsigned. */
#define BOARD_PIN_NONE 0xFF

typedef struct {
    const char *id;             /* stable id, used as NVS key value */
    const char *display;        /* human label shown in UI */
    const char *mcu;             /* "esp32c3", "esp32", "esp32s3", "esp32c6" */
    bool        validated;       /* true = hardware-tested, false = ships untested */

    /* Sane defaults — the board ships with these unless the user remaps. */
    uint8_t led_pin;
    uint8_t radar_rx_pin;        /* MCU side: receives from radar TX */
    uint8_t radar_tx_pin;        /* MCU side: transmits to radar RX */
    uint8_t button_pin;
    uint8_t status_led_pin;      /* on-board LED, often inverted-active-low */
    bool    status_led_active_low;

    /* Peripheral defaults */
    uint8_t uart_num;            /* UART number for radar — 1 on most boards */
    uint8_t rmt_channel;         /* RMT channel for led_strip */

    /* Bitmask of GPIOs that must NOT be remapped to user I/O.
     * Bit N = 1 → GPIO N is unsafe (strapping, flash, USB, reserved). */
    uint64_t unsafe_pin_mask;

    /* Highest legal GPIO number on this part — UI clamps dropdowns to this. */
    uint8_t max_gpio;
} board_profile_t;

/* Return the array of all known profiles and its length. */
const board_profile_t *board_profiles(size_t *out_count);

/* Look up a profile by id; returns NULL if id is unknown. */
const board_profile_t *board_profile_by_id(const char *id);

/* The compile-time default profile for this build's CONFIG_IDF_TARGET.
 * Used on first boot when NVS has no board.id key yet. */
const board_profile_t *board_default_profile(void);

/* True if `pin` is in `profile`'s unsafe_pin_mask. */
bool board_pin_is_unsafe(const board_profile_t *profile, uint8_t pin);

#ifdef __cplusplus
}
#endif
