#include "presence.h"

#include <string.h>
#include <stdlib.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "radar.h"
#include "settings.h"

static const char *TAG = "presence";

#define PRESENCE_TICK_MS         100   /* 10 Hz */
#define PRESENCE_STATIONARY_CMS  5     /* |v| < 5 cm/s == stationary */
#define VACANCY_DEFAULT_S        60
#define VACANCY_MIN_S            5
#define VACANCY_MAX_S            3600

static struct {
    presence_t latest;
    SemaphoreHandle_t lock;
    uint32_t vacancy_secs;
    /* Latest detection timestamp in microseconds since boot.
     * 0 means "never seen". */
    uint64_t last_seen_us;
    bool inited;
    TaskHandle_t task;
} s_pres;

static void presence_task(void *arg) {
    (void)arg;
    while (1) {
        radar_frame_t f;
        esp_err_t r = radar_peek(&f);
        uint64_t now_us = (uint64_t)esp_timer_get_time();

        bool any_now = false;
        bool any_stationary = false;
        int16_t nearest = -1;
        uint8_t count = 0;

        if (r == ESP_OK) {
            count = f.target_count;
            any_now = (f.target_count > 0) || f.present;

            /* `f.distance_cm` is the EUCLIDEAN distance the LD2450 driver
             * computes from the primary target's (x, y) — same number
             * the Live tab shows. The per-target `targets[i].y_cm` is
             * just the Y axis projection, NOT the radial distance, so
             * using it as "nearest" reports 0 when a target sits along
             * the X axis. (Bench-reproduced 2026-05-07.)
             *
             * For LD2410-family, distance_cm is just the moving or
             * stationary distance the radar reports. Same field, same
             * meaning. So both code paths reduce to: "if anyone is here,
             * nearest = f.distance_cm". */
            if (any_now) {
                nearest = (int16_t)f.distance_cm;
                if (count == 0 && f.present) count = 1;

                /* Stationary detection: LD2450 reports per-target
                 * velocity in cm/s; LD2410-family doesn't surface
                 * velocity through the unified frame at all. Mark
                 * stationary if any target has |v| under threshold,
                 * OR if we don't know (LD2410 case — bias toward
                 * stationary since the family is tuned for it). */
                if (f.target_count > 0) {
                    for (uint8_t i = 0; i < f.target_count && i < RADAR_MAX_TARGETS; ++i) {
                        if (abs((int)f.targets[i].v_cms) < PRESENCE_STATIONARY_CMS) {
                            any_stationary = true;
                            break;
                        }
                    }
                } else {
                    any_stationary = true;  /* LD2410(C) — unknown velocity */
                }

                s_pres.last_seen_us = now_us;
            }
        }

        uint64_t timeout_us = (uint64_t)s_pres.vacancy_secs * 1000000ULL;
        bool occupied = (s_pres.last_seen_us != 0) &&
                        ((now_us - s_pres.last_seen_us) < timeout_us);

        presence_t snap = {
            .occupied      = occupied,
            .target_count  = occupied ? count : 0,
            .stationary    = occupied && any_stationary,
            .nearest_cm    = occupied ? (nearest < 0 ? 0 : nearest) : -1,
            .last_seen_ms  = (uint32_t)(s_pres.last_seen_us / 1000ULL),
            .ms_since_seen = (s_pres.last_seen_us == 0) ? UINT32_MAX
                             : (uint32_t)((now_us - s_pres.last_seen_us) / 1000ULL),
            .vacancy_secs  = s_pres.vacancy_secs,
        };

        xSemaphoreTake(s_pres.lock, portMAX_DELAY);
        s_pres.latest = snap;
        xSemaphoreGive(s_pres.lock);

        vTaskDelay(pdMS_TO_TICKS(PRESENCE_TICK_MS));
    }
}

esp_err_t presence_init(void) {
    if (s_pres.inited) return ESP_OK;
    s_pres.lock = xSemaphoreCreateMutex();

    /* Load vacancy timeout from NVS or fall back to default. */
    uint32_t v = 0;
    if (settings_get_u32("presence", "vacancy", &v) == ESP_OK &&
        v >= VACANCY_MIN_S && v <= VACANCY_MAX_S) {
        s_pres.vacancy_secs = v;
    } else {
        s_pres.vacancy_secs = VACANCY_DEFAULT_S;
    }
    s_pres.last_seen_us = 0;
    /* Initial latest snapshot — vacant, never seen. */
    s_pres.latest = (presence_t){
        .occupied = false, .target_count = 0, .stationary = false,
        .nearest_cm = -1, .last_seen_ms = 0, .ms_since_seen = UINT32_MAX,
        .vacancy_secs = s_pres.vacancy_secs,
    };

    BaseType_t ok = xTaskCreate(presence_task, "presence", 3072, NULL, 4, &s_pres.task);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "task create failed");
        return ESP_FAIL;
    }
    s_pres.inited = true;
    ESP_LOGI(TAG, "presence detection up; vacancy timeout %u s", (unsigned)s_pres.vacancy_secs);
    return ESP_OK;
}

esp_err_t presence_get(presence_t *out) {
    if (!s_pres.inited || !out) return ESP_ERR_INVALID_STATE;
    xSemaphoreTake(s_pres.lock, portMAX_DELAY);
    *out = s_pres.latest;
    xSemaphoreGive(s_pres.lock);
    return ESP_OK;
}

esp_err_t presence_set_vacancy_timeout(uint32_t secs) {
    if (secs < VACANCY_MIN_S || secs > VACANCY_MAX_S) return ESP_ERR_INVALID_ARG;
    s_pres.vacancy_secs = secs;
    settings_set_u32("presence", "vacancy", secs);
    ESP_LOGI(TAG, "vacancy timeout set to %u s", (unsigned)secs);
    return ESP_OK;
}

uint32_t presence_get_vacancy_timeout(void) {
    return s_pres.inited ? s_pres.vacancy_secs : VACANCY_DEFAULT_S;
}
