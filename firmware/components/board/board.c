#include "board.h"
#include "sdkconfig.h"
#include <string.h>

/*
 * Pin safety masks were derived from the ESP32-{C3,C6,S3,base} hardware
 * design guidelines.  When in doubt, prefer marking a pin unsafe — users
 * can edit board.c if they know what they're doing.
 *
 * UNSAFE(n) is bit n in a uint64_t.  Helper here for readability.
 */
#define U(n) (1ULL << (n))

/* ESP32-C3 SuperMini
 *   - GPIO 8: boot strap, also onboard LED (active-low) on most clones
 *   - GPIO 9: boot strap (download mode), pulled up — driving low at boot bricks boot
 *   - GPIO 11: SPI VDD strapping
 *   - GPIO 12-17: SPI flash on internal-flash variants
 *   - GPIO 18, 19: USB-Serial-JTAG D-/D+
 * Status LED on this board is GPIO 8 (active-low). We *do* use it for status,
 * which is fine — we're not remapping it to anything else.
 * Radar UART on GPIO 20 (RX) and GPIO 21 (TX) keeps us off USB and strapping.
 * LED data on GPIO 10 — middle of the safe block.
 * Button on GPIO 4.
 */
static const board_profile_t profile_c3_supermini = {
    .id            = "esp32c3-supermini",
    .display       = "ESP32-C3 SuperMini",
    .mcu           = "esp32c3",
    .validated     = true,
    .led_pin       = 10,
    .radar_rx_pin  = 20,
    .radar_tx_pin  = 21,
    .button_pin    = 4,
    .status_led_pin = 8,
    .status_led_active_low = true,
    .uart_num      = 1,
    .rmt_channel   = 0,
    .unsafe_pin_mask =
        U(9)  | U(11) |                  /* strapping */
        U(12) | U(13) | U(14) | U(15) | U(16) | U(17) |  /* internal flash */
        U(18) | U(19),                   /* USB-Serial-JTAG */
    .max_gpio = 21,
};

/* Classic ESP32 DevKit (WROOM-32, 30 or 38 pin)
 *   - GPIO 0, 2, 5, 12, 15: strapping pins
 *   - GPIO 6-11: internal SPI flash — NEVER touch
 *   - GPIO 1, 3: UART0 console (we keep it for log output by default)
 *   - GPIO 34-39: input-only (not usable for LED data or radar TX)
 * Defaults match the original AmbiSense Arduino mapping where they don't
 * conflict.
 */
static const board_profile_t profile_esp32_devkit = {
    .id            = "esp32-devkit",
    .display       = "ESP32 DevKit (WROOM-32)",
    .mcu           = "esp32",
    .validated     = false,
    .led_pin       = 5,
    .radar_rx_pin  = 16,
    .radar_tx_pin  = 17,
    .button_pin    = 4,
    .status_led_pin = 2,
    .status_led_active_low = false,
    .uart_num      = 2,
    .rmt_channel   = 0,
    .unsafe_pin_mask =
        U(0)  | U(2)  |                  /* strapping (5/12/15 are usable with care) */
        U(1)  | U(3)  |                  /* UART0 console */
        U(6)  | U(7)  | U(8)  | U(9)  | U(10) | U(11) |  /* SPI flash */
        U(20) | U(24) | U(28) | U(29) | U(30) | U(31),   /* not bonded */
    .max_gpio = 39,
};

/* ESP32-S3 (Zero / SuperMini class)
 *   - GPIO 0, 3, 45, 46: strapping
 *   - GPIO 19, 20: USB-Serial-JTAG
 *   - GPIO 26-32: SPI flash on the WROOM module variants
 *   - GPIO 33-37: octal PSRAM/flash — depends on module
 * Defaults pick visible header pins on common Zero-class boards.
 */
static const board_profile_t profile_esp32s3_zero = {
    .id            = "esp32s3-zero",
    .display       = "ESP32-S3 Zero/SuperMini",
    .mcu           = "esp32s3",
    .validated     = false,
    .led_pin       = 21,                 /* common WS2812 default on S3 Zero */
    .radar_rx_pin  = 4,
    .radar_tx_pin  = 5,
    .button_pin    = 9,
    .status_led_pin = 21,                /* same pin as data on some Zero clones; user should remap */
    .status_led_active_low = false,
    .uart_num      = 1,
    .rmt_channel   = 0,
    .unsafe_pin_mask =
        U(0)  | U(3)  | U(45) | U(46) |  /* strapping */
        U(19) | U(20) |                  /* USB-Serial-JTAG */
        U(26) | U(27) | U(28) | U(29) | U(30) | U(31) | U(32) |  /* flash */
        U(33) | U(34) | U(35) | U(36) | U(37),   /* octal flash/PSRAM (WROOM-1) */
    .max_gpio = 48,
};

/* ESP32-C6 DevKit
 *   - GPIO 8, 9: strapping
 *   - GPIO 12, 13: USB-Serial-JTAG
 *   - GPIO 24-30: internal SPI flash on most modules
 * C6 also has 802.15.4 (Thread/Zigbee/Matter) — future-relevant but unused
 * for v6.
 */
static const board_profile_t profile_esp32c6_devkit = {
    .id            = "esp32c6-devkit",
    .display       = "ESP32-C6 DevKit",
    .mcu           = "esp32c6",
    .validated     = false,
    .led_pin       = 8,                  /* onboard WS2812 on most C6 devkits */
    .radar_rx_pin  = 4,
    .radar_tx_pin  = 5,
    .button_pin    = 9,
    .status_led_pin = 15,
    .status_led_active_low = false,
    .uart_num      = 1,
    .rmt_channel   = 0,
    .unsafe_pin_mask =
        U(9)  |                          /* strap */
        U(12) | U(13) |                  /* USB-Serial-JTAG */
        U(24) | U(25) | U(26) | U(27) | U(28) | U(29) | U(30),  /* flash */
    .max_gpio = 30,
};

static const board_profile_t *const k_all_profiles[] = {
    &profile_c3_supermini,
    &profile_esp32_devkit,
    &profile_esp32s3_zero,
    &profile_esp32c6_devkit,
};

/* The order in which we present profiles to the UI. C3 first because it's
 * the validated default for v6.0. */
const board_profile_t *board_profiles(size_t *out_count) {
    if (out_count) {
        *out_count = sizeof(k_all_profiles) / sizeof(k_all_profiles[0]);
    }
    /* The pointer-array layout is what callers iterate; we expose the
     * underlying first-profile address as the base. */
    return *k_all_profiles;
}

const board_profile_t *board_profile_by_id(const char *id) {
    if (!id) return NULL;
    for (size_t i = 0; i < sizeof(k_all_profiles) / sizeof(k_all_profiles[0]); ++i) {
        if (strcmp(k_all_profiles[i]->id, id) == 0) {
            return k_all_profiles[i];
        }
    }
    return NULL;
}

const board_profile_t *board_default_profile(void) {
    /* Match the build's CONFIG_IDF_TARGET to a profile id. */
#if CONFIG_IDF_TARGET_ESP32C3
    return &profile_c3_supermini;
#elif CONFIG_IDF_TARGET_ESP32
    return &profile_esp32_devkit;
#elif CONFIG_IDF_TARGET_ESP32S3
    return &profile_esp32s3_zero;
#elif CONFIG_IDF_TARGET_ESP32C6
    return &profile_esp32c6_devkit;
#else
    return &profile_c3_supermini;
#endif
}

bool board_pin_is_unsafe(const board_profile_t *profile, uint8_t pin) {
    if (!profile) return true;
    if (pin > profile->max_gpio) return true;
    return (profile->unsafe_pin_mask & U(pin)) != 0;
}
