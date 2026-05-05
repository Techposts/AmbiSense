#include "motion.h"

#include <string.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "radar.h"
#include "settings.h"

static const char *TAG = "motion";

/* Defaults match v5 (config.h:69-76). NVS values stored as x1000 fixed-point. */
static struct {
    bool     enabled;
    float    pos_smooth;
    float    vel_smooth;
    float    predict;
    float    p_gain;
    float    i_gain;

    float    smoothed;
    float    predicted;
    float    velocity;
    float    err_integral;
    uint64_t last_us;

    target_t latest;
    SemaphoreHandle_t lock;
    int      min_cm, max_cm;
} s_m;

static float clamp(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static void motion_task(void *arg) {
    (void)arg;
    radar_frame_t f;
    while (1) {
        if (radar_read(&f, pdMS_TO_TICKS(1000)) != ESP_OK) {
            target_t t = {0};
            xSemaphoreTake(s_m.lock, portMAX_DELAY);
            t = s_m.latest;
            t.present = false;
            s_m.latest = t;
            xSemaphoreGive(s_m.lock);
            continue;
        }

        int raw = f.distance_cm;
        if (raw < s_m.min_cm) raw = s_m.min_cm;
        if (raw > s_m.max_cm) raw = s_m.max_cm;

        target_t t = { .present = f.present, .energy = f.energy,
                       .direction = f.direction, .ts_us = f.ts_us };

        if (!s_m.enabled) {
            t.distance_cm = (int16_t)raw;
        } else {
            uint64_t now = f.ts_us;
            float dt = s_m.last_us ? (float)(now - s_m.last_us) / 1e6f : 0.02f;
            s_m.last_us = now;
            dt = clamp(dt, 0.001f, 1.0f);

            if (s_m.smoothed <= 0) {
                s_m.smoothed = (float)raw;
                s_m.predicted = (float)raw;
            }

            s_m.smoothed = (1.f - s_m.pos_smooth) * s_m.smoothed +
                                  s_m.pos_smooth  * (float)raw;
            float instant_v = (s_m.smoothed - s_m.predicted) / dt;
            instant_v = clamp(instant_v, -200.f, 200.f);
            s_m.velocity = (1.f - s_m.vel_smooth) * s_m.velocity +
                                  s_m.vel_smooth  * instant_v;
            s_m.predicted = s_m.smoothed + s_m.velocity * s_m.predict;
            float perr = s_m.predicted - s_m.smoothed;
            s_m.err_integral = clamp(s_m.err_integral + perr * dt, -100.f, 100.f);
            float ctl = s_m.p_gain * perr + s_m.i_gain * s_m.err_integral;
            int final_d = (int)(s_m.predicted + ctl);
            if (final_d < s_m.min_cm) final_d = s_m.min_cm;
            if (final_d > s_m.max_cm) final_d = s_m.max_cm;
            t.distance_cm = (int16_t)final_d;
        }

        xSemaphoreTake(s_m.lock, portMAX_DELAY);
        s_m.latest = t;
        xSemaphoreGive(s_m.lock);
    }
}

esp_err_t motion_init(void) {
    s_m.lock = xSemaphoreCreateMutex();

    /* Load tunables. NVS values are stored ×1000 to keep them integer. */
    uint8_t en = 1; settings_get_u8("motion", "en", &en); s_m.enabled = en != 0;
    uint32_t v = 200; settings_get_u32("motion", "ps", &v); s_m.pos_smooth = v / 1000.f;
    v = 100; settings_get_u32("motion", "vs", &v); s_m.vel_smooth = v / 1000.f;
    v = 500; settings_get_u32("motion", "pf", &v); s_m.predict   = v / 1000.f;
    v = 100; settings_get_u32("motion", "pg", &v); s_m.p_gain    = v / 1000.f;
    v = 10;  settings_get_u32("motion", "ig", &v); s_m.i_gain    = v / 1000.f;

    uint32_t mn = 30, mx = 300;
    settings_get_u32("dist", "min", &mn);
    settings_get_u32("dist", "max", &mx);
    s_m.min_cm = (int)mn; s_m.max_cm = (int)mx;
    if (s_m.max_cm <= s_m.min_cm) { s_m.min_cm = 30; s_m.max_cm = 300; }

    ESP_LOGI(TAG, "Smoother: en=%d ps=%.2f vs=%.2f pf=%.2f pg=%.2f ig=%.2f range=%d..%d cm",
             s_m.enabled, s_m.pos_smooth, s_m.vel_smooth, s_m.predict,
             s_m.p_gain, s_m.i_gain, s_m.min_cm, s_m.max_cm);

    xTaskCreate(motion_task, "motion", 4096, NULL, 5, NULL);
    return ESP_OK;
}

void motion_get(target_t *out) {
    if (!out) return;
    xSemaphoreTake(s_m.lock, portMAX_DELAY);
    *out = s_m.latest;
    xSemaphoreGive(s_m.lock);
}
