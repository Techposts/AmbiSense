#include "button.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "button";

#define POLL_MS         20
#define DEBOUNCE_MS     40
#define LONG_MS         3000
#define VERYLONG_MS     10000

static struct {
    uint8_t           gpio;
    bool              active_low;
    button_event_cb_t cb;
} s_b;

static inline bool read_pressed(void) {
    int level = gpio_get_level(s_b.gpio);
    return s_b.active_low ? (level == 0) : (level != 0);
}

static void button_task(void *arg) {
    (void)arg;
    bool pressed = false;
    uint64_t press_started_us = 0;
    int debounce_ticks = 0;
    bool fired_long = false;
    bool fired_verylong = false;

    while (1) {
        bool now_pressed = read_pressed();

        if (now_pressed != pressed) {
            /* Edge detected — but don't trust it until DEBOUNCE_MS of
             * matching reads have stacked up. Counter approach is simpler
             * (and has identical effect to) a 40-ms blocking sleep. */
            debounce_ticks += POLL_MS;
            if (debounce_ticks >= DEBOUNCE_MS) {
                pressed = now_pressed;
                debounce_ticks = 0;
                if (pressed) {
                    press_started_us = (uint64_t)esp_timer_get_time();
                    fired_long = false;
                    fired_verylong = false;
                    ESP_LOGD(TAG, "press start");
                } else {
                    /* Released — fire short if no long fired. */
                    uint64_t held_ms = ((uint64_t)esp_timer_get_time() - press_started_us) / 1000ULL;
                    ESP_LOGI(TAG, "press end after %llu ms", (unsigned long long)held_ms);
                    if (!fired_long && !fired_verylong && held_ms < LONG_MS && held_ms >= 50) {
                        if (s_b.cb) s_b.cb(BUTTON_PRESS_SHORT);
                    }
                }
            }
        } else {
            debounce_ticks = 0;
            /* While held, fire long/verylong on threshold crossing. */
            if (pressed && !fired_long) {
                uint64_t held_ms = ((uint64_t)esp_timer_get_time() - press_started_us) / 1000ULL;
                if (held_ms >= LONG_MS) {
                    fired_long = true;
                    ESP_LOGI(TAG, "long press @ %llu ms", (unsigned long long)held_ms);
                    if (s_b.cb) s_b.cb(BUTTON_PRESS_LONG);
                }
            }
            if (pressed && fired_long && !fired_verylong) {
                uint64_t held_ms = ((uint64_t)esp_timer_get_time() - press_started_us) / 1000ULL;
                if (held_ms >= VERYLONG_MS) {
                    fired_verylong = true;
                    ESP_LOGI(TAG, "very-long press @ %llu ms", (unsigned long long)held_ms);
                    if (s_b.cb) s_b.cb(BUTTON_PRESS_VERYLONG);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(POLL_MS));
    }
}

esp_err_t button_init(uint8_t gpio_num, bool active_low, button_event_cb_t cb) {
    s_b.gpio = gpio_num;
    s_b.active_low = active_low;
    s_b.cb = cb;

    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << gpio_num,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en   = active_low ? GPIO_PULLUP_ENABLE   : GPIO_PULLUP_DISABLE,
        .pull_down_en = active_low ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "gpio_config(%u) failed: 0x%x", gpio_num, err);
        return err;
    }

    BaseType_t ok = xTaskCreate(button_task, "button", 2048, NULL, 4, NULL);
    if (ok != pdPASS) return ESP_ERR_NO_MEM;

    ESP_LOGI(TAG, "button on GPIO %u (active_%s)", gpio_num, active_low ? "low" : "high");
    return ESP_OK;
}
