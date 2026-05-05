#include "status_led.h"

#include <stdatomic.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "status_led";

static struct {
    uint8_t  gpio;
    bool     active_low;
    bool     inited;
    _Atomic uint32_t pattern;     /* status_led_pattern_t cast to uint32 for atomic ops */
} s_led;

static inline void led_set(bool on) {
    int level = on ? (s_led.active_low ? 0 : 1)
                   : (s_led.active_low ? 1 : 0);
    gpio_set_level(s_led.gpio, level);
}

/* Each pattern is a sequence of (level, duration_ms) steps, repeated. We
 * encode them inline rather than table-driven so the patterns are easy to
 * read and tweak. */
static void run_pattern(status_led_pattern_t p) {
    switch (p) {
        case STATUS_LED_OFF:
            led_set(false);
            vTaskDelay(pdMS_TO_TICKS(200));
            return;

        case STATUS_LED_BOOT:
            led_set(true);
            vTaskDelay(pdMS_TO_TICKS(200));
            return;

        case STATUS_LED_AP_MODE:
            led_set(true);  vTaskDelay(pdMS_TO_TICKS(500));
            led_set(false); vTaskDelay(pdMS_TO_TICKS(500));
            return;

        case STATUS_LED_STA_MODE: {
            /* Heartbeat: two short pulses then a longer dark pause. */
            led_set(true);  vTaskDelay(pdMS_TO_TICKS(80));
            led_set(false); vTaskDelay(pdMS_TO_TICKS(120));
            led_set(true);  vTaskDelay(pdMS_TO_TICKS(80));
            led_set(false); vTaskDelay(pdMS_TO_TICKS(1720));
            return;
        }

        case STATUS_LED_OTA:
            led_set(true);  vTaskDelay(pdMS_TO_TICKS(100));
            led_set(false); vTaskDelay(pdMS_TO_TICKS(100));
            return;

        case STATUS_LED_ERROR:
            led_set(true);  vTaskDelay(pdMS_TO_TICKS(600));
            led_set(false); vTaskDelay(pdMS_TO_TICKS(150));
            led_set(true);  vTaskDelay(pdMS_TO_TICKS(150));
            led_set(false); vTaskDelay(pdMS_TO_TICKS(150));
            led_set(true);  vTaskDelay(pdMS_TO_TICKS(600));
            led_set(false); vTaskDelay(pdMS_TO_TICKS(800));
            return;

        case STATUS_LED_PANIC: {
            /* SOS · · · — — — · · · */
            for (int i = 0; i < 3; ++i) {
                led_set(true);  vTaskDelay(pdMS_TO_TICKS(120));
                led_set(false); vTaskDelay(pdMS_TO_TICKS(180));
            }
            vTaskDelay(pdMS_TO_TICKS(200));
            for (int i = 0; i < 3; ++i) {
                led_set(true);  vTaskDelay(pdMS_TO_TICKS(360));
                led_set(false); vTaskDelay(pdMS_TO_TICKS(180));
            }
            vTaskDelay(pdMS_TO_TICKS(200));
            for (int i = 0; i < 3; ++i) {
                led_set(true);  vTaskDelay(pdMS_TO_TICKS(120));
                led_set(false); vTaskDelay(pdMS_TO_TICKS(180));
            }
            vTaskDelay(pdMS_TO_TICKS(800));
            return;
        }
    }
}

static void status_led_task(void *arg) {
    (void)arg;
    while (1) {
        status_led_pattern_t p = (status_led_pattern_t)atomic_load(&s_led.pattern);
        run_pattern(p);
    }
}

esp_err_t status_led_init(uint8_t gpio_num, bool active_low) {
    if (s_led.inited) return ESP_OK;

    s_led.gpio = gpio_num;
    s_led.active_low = active_low;
    atomic_store(&s_led.pattern, (uint32_t)STATUS_LED_BOOT);

    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << gpio_num,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "gpio_config(%u) failed: 0x%x", gpio_num, err);
        return err;
    }
    led_set(false);

    BaseType_t ok = xTaskCreate(status_led_task, "status_led", 2048, NULL, 2, NULL);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "task create failed");
        return ESP_ERR_NO_MEM;
    }

    s_led.inited = true;
    ESP_LOGI(TAG, "status LED on GPIO %u (active_%s)", gpio_num, active_low ? "low" : "high");
    return ESP_OK;
}

void status_led_set_pattern(status_led_pattern_t pattern) {
    atomic_store(&s_led.pattern, (uint32_t)pattern);
}
