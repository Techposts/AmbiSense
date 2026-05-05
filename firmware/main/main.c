/*
 * AmbiSense v6 — application entry.
 *
 * Boot order:
 *   1. settings_init  — bring up NVS (replaces v5 EEPROM)
 *   2. resolve board  — NVS-saved board id wins over compile-time default
 *   3. status LED     — boot pattern; flips later as Wi-Fi / OTA progresses
 *
 * Subsequent PRs add Wi-Fi (PR #2), web server (PR #2), radar (PR #3),
 * LED engine (PR #3), peer mesh (PR #4), and the real UI (PR #5).
 */

#include <string.h>

#include "esp_log.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_app_desc.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "board.h"
#include "settings.h"
#include "status_led.h"
#include "netmgr.h"
#include "auth.h"
#include "webui.h"
#include "ota.h"
#include "radar.h"
#include "motion.h"
#include "led_engine.h"
#include "topology.h"
#include "mesh.h"

static const char *TAG = "ambisense";

/* Resolve which board_profile_t this device should use:
 *  1. If NVS has board.id set, look it up by id.
 *  2. Otherwise, fall back to the build's compile-time default.
 *  3. If a corrupt/unknown id was somehow stored, log and fall back.
 */
static const board_profile_t *resolve_board_profile(void) {
    char saved_id[32] = {0};
    esp_err_t err = settings_get_board_id(saved_id, sizeof(saved_id));
    if (err == ESP_OK) {
        const board_profile_t *p = board_profile_by_id(saved_id);
        if (p) {
            ESP_LOGI(TAG, "Board profile from NVS: %s (%s)", p->id, p->display);
            return p;
        }
        ESP_LOGW(TAG, "NVS board.id='%s' is unknown; falling back to default", saved_id);
    }
    const board_profile_t *def = board_default_profile();
    ESP_LOGI(TAG, "Board profile (default): %s (%s)", def->id, def->display);
    return def;
}

/* Apply pin overrides from NVS on top of the profile's defaults.
 * Each override is silently ignored if it would land on an unsafe pin. */
static void apply_pin_overrides(board_profile_t *runtime) {
    static const struct { const char *key; size_t off; } pin_keys[] = {
        { "led_pin",     offsetof(board_profile_t, led_pin)        },
        { "radar_rx",    offsetof(board_profile_t, radar_rx_pin)   },
        { "radar_tx",    offsetof(board_profile_t, radar_tx_pin)   },
        { "button",      offsetof(board_profile_t, button_pin)     },
        { "status_led",  offsetof(board_profile_t, status_led_pin) },
    };
    for (size_t i = 0; i < sizeof(pin_keys) / sizeof(pin_keys[0]); ++i) {
        uint8_t pin;
        if (settings_get_pin_override(pin_keys[i].key, &pin) != ESP_OK) continue;
        if (pin == BOARD_PIN_NONE) continue;
        if (board_pin_is_unsafe(runtime, pin)) {
            ESP_LOGW(TAG, "Ignoring NVS pin override %s=%u (unsafe on this board)",
                     pin_keys[i].key, pin);
            continue;
        }
        *(uint8_t *)((char *)runtime + pin_keys[i].off) = pin;
        ESP_LOGI(TAG, "Pin override: %s = GPIO %u", pin_keys[i].key, pin);
    }
}

/* Telemetry pump: 5 Hz publish mesh-fused target + RSSI + peer health to
 * webui WS clients. Falls back to local motion_get() if mesh has no peers. */
static void telemetry_pump_task(void *arg) {
    (void)arg;
    while (1) {
        mesh_fused_t f;
        mesh_get_fused(&f);
        mesh_peer_t peers[MESH_MAX_PEERS];
        size_t pn = mesh_peers_snapshot(peers, MESH_MAX_PEERS);
        size_t hn = 0;
        for (size_t i = 0; i < pn; ++i) if (peers[i].healthy) hn++;

        webui_live_t live = {
            .distance_cm = f.distance_cm,
            .direction   = f.direction,
            .rssi        = netmgr_get_rssi(),
            .free_heap   = 0,   /* webui fills these in itself before broadcast */
            .uptime_s    = 0,
            .peer_count   = (uint8_t)pn,
            .peer_healthy = (uint8_t)hn,
        };
        webui_publish_live(&live);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

static void log_chip_info(void) {
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    const esp_app_desc_t *app = esp_app_get_description();
    ESP_LOGI(TAG, "AmbiSense v6 — %s, IDF %s, built %s %s",
             app->version, app->idf_ver, app->date, app->time);
    ESP_LOGI(TAG, "MCU: %s rev v%d.%d, %d core(s), MAC %02X:%02X:%02X:%02X:%02X:%02X",
             CONFIG_IDF_TARGET, chip.revision / 100, chip.revision % 100, chip.cores,
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void app_main(void) {
    log_chip_info();

    if (settings_init() != ESP_OK) {
        ESP_LOGE(TAG, "NVS init failed — settings will not persist this boot");
    }

    /* Copy the const profile into a mutable runtime struct so pin overrides
     * apply only to this boot's working copy. */
    board_profile_t runtime = *resolve_board_profile();
    apply_pin_overrides(&runtime);

    ESP_ERROR_CHECK(status_led_init(runtime.status_led_pin, runtime.status_led_active_low));
    status_led_set_pattern(STATUS_LED_BOOT);

    ESP_LOGI(TAG, "Pins: led=%u radar_rx=%u radar_tx=%u button=%u status=%u (uart%u, rmt%u)",
             runtime.led_pin, runtime.radar_rx_pin, runtime.radar_tx_pin,
             runtime.button_pin, runtime.status_led_pin,
             runtime.uart_num, runtime.rmt_channel);

    /* Auth (off until password set), Wi-Fi (always-on AP + optional STA),
     * web server (port 80, every API endpoint plus captive portal). */
    auth_init();
    netmgr_init();
    webui_init();

    /* Radar + motion smoother + LED engine. The render task pulls smoothed
     * targets from motion_q and drives the strip at 60 Hz; the radar task
     * parses UART frames; the motion task runs the PI smoother in between. */
    radar_config_t rcfg = {
        .uart_num = runtime.uart_num,
        .rx_pin   = runtime.radar_rx_pin,
        .tx_pin   = runtime.radar_tx_pin,
        .baud     = 256000,
    };
    if (radar_init(&rcfg) != ESP_OK) ESP_LOGW(TAG, "radar_init failed (continuing)");
    motion_init();
    if (led_engine_init(runtime.led_pin) != ESP_OK) {
        ESP_LOGE(TAG, "led_engine_init on GPIO %u failed", runtime.led_pin);
    }

    /* Topology + ESP-NOW peer mesh. Comes after netmgr because esp_now_init
     * requires Wi-Fi started. */
    topology_init();
    if (mesh_init() != ESP_OK) ESP_LOGW(TAG, "mesh_init failed (single-device fallback)");

    xTaskCreate(telemetry_pump_task, "tele_pump", 3072, NULL, 3, NULL);

    /* If we're running on a freshly-flashed image with rollback armed, mark
     * us valid so the bootloader doesn't revert on next reset. */
    ota_mark_valid();

    /* Status LED follows Wi-Fi state from here. */
    status_led_set_pattern(netmgr_is_sta_connected() ? STATUS_LED_STA_MODE : STATUS_LED_AP_MODE);

    ESP_LOGI(TAG, "Boot complete. Web UI on http://%s.local/ (when STA up) or AP \"AmbiSense-XXXX\" → 192.168.4.1.", "ambisense");
    /* app_main returns; FreeRTOS owns the device. */
}
