#include "motion.h"
#include "motion_kalman.h"

#include <string.h>
#include <math.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "radar.h"
#include "settings.h"

static const char *TAG = "motion";

/* Outlier (median) buffer: up to 7 samples for the "Strong" setting. */
#define MEDIAN_MAX_W 7

typedef enum { MODE_KALMAN = 0, MODE_PI = 1 } motion_mode_t;

static struct {
    motion_mode_t mode;

    /* User-facing simplified knobs. */
    bool     enabled;
    uint8_t  response;        /* 0..100 — calm⇄snappy */
    uint16_t look_ahead_ms;   /* 0..500 — predictive lead */
    uint8_t  outlier_strength; /* 0=off, 1=soft (3), 2=strong (7) */

    /* Legacy / advanced PI knobs (still respected if present in NVS). */
    float    pos_smooth;
    float    vel_smooth;
    float    predict;
    float    p_gain;
    float    i_gain;

    /* PI smoother state. */
    float    smoothed;
    float    predicted;
    float    velocity;
    float    err_integral;

    /* Kalman state. */
    kalman_t kf;

    /* Common. */
    uint64_t last_us;
    int16_t  med_buf[MEDIAN_MAX_W];
    uint8_t  med_idx;
    uint8_t  med_filled;

    target_t latest;
    SemaphoreHandle_t lock;
    int      min_cm, max_cm;
} s_m;

static float clamp(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

/* Median over `w` samples drawn (most recent w) from `src` ring buffer.
 * `w` must be ≤ MEDIAN_MAX_W and odd. */
static int16_t median_w(int16_t *src, uint8_t idx, uint8_t filled, uint8_t w) {
    if (w <= 1 || filled < w) {
        /* Not enough samples yet — fall back to most recent. */
        uint8_t last = (idx + MEDIAN_MAX_W - 1) % MEDIAN_MAX_W;
        return src[last];
    }
    int16_t a[MEDIAN_MAX_W];
    /* Copy last w samples, walking backwards from idx-1. */
    for (uint8_t i = 0; i < w; ++i) {
        uint8_t k = (idx + MEDIAN_MAX_W - 1 - i) % MEDIAN_MAX_W;
        a[i] = src[k];
    }
    for (int i = 1; i < w; ++i) {
        int16_t x = a[i]; int j = i - 1;
        while (j >= 0 && a[j] > x) { a[j+1] = a[j]; j--; }
        a[j+1] = x;
    }
    return a[w / 2];
}

static uint8_t outlier_window(uint8_t strength) {
    return (strength == 2) ? 7 : (strength == 1) ? 3 : 1;
}

/* Map the 0..100 user "Response" knob to Kalman process noise. The curve
 * is exponential so the slider has perceptual linearity — at 50 the
 * tracker feels balanced, at 0 it's heavy, at 100 it's nearly raw. */
static void response_to_kalman_q(uint8_t response, float *Q_pos, float *Q_vel) {
    float r = (float)response / 100.f;       /* 0..1 */
    /* Q_pos: 0.1 .. 4 cm²/s   (exponential)
     * Q_vel: 2   .. 80 cm²/s³ (exponential) */
    *Q_pos = 0.1f * powf(40.f, r);          /* 0.1 → 4 */
    *Q_vel = 2.f  * powf(40.f, r);          /* 2   → 80 */
}

/* Same response slider drives the PI's pos_smooth alpha when mode == PI. */
static float response_to_pi_alpha(uint8_t response) {
    /* 0 → 0.05 (very calm), 50 → 0.20 (default), 100 → 0.6 (very snappy) */
    float r = (float)response / 100.f;
    return 0.05f + 0.55f * r;
}

static void run_pi(float filtered_raw, float dt, target_t *t) {
    if (s_m.smoothed <= 0) {
        s_m.smoothed = filtered_raw;
        s_m.predicted = filtered_raw;
    }
    /* Adaptive alpha: ease the filter when target is moving fast. The
     * envelope is (pos_smooth .. min(0.9, pos_smooth*4)). Same logic v2
     * had — the PI path is the "Legacy" mode users select for parity. */
    float delta = fabsf(filtered_raw - s_m.smoothed);
    float scale = clamp(delta / 30.f, 0.f, 1.f);
    float alpha_max = clamp(s_m.pos_smooth * 4.f, s_m.pos_smooth, 0.9f);
    float alpha_eff = s_m.pos_smooth + (alpha_max - s_m.pos_smooth) * scale;

    s_m.smoothed = (1.f - alpha_eff) * s_m.smoothed + alpha_eff * filtered_raw;
    float instant_v = (s_m.smoothed - s_m.predicted) / dt;
    instant_v = clamp(instant_v, -200.f, 200.f);
    s_m.velocity = (1.f - s_m.vel_smooth) * s_m.velocity + s_m.vel_smooth * instant_v;
    /* User-facing look-ahead-ms overrides the legacy `predict` knob if set. */
    float predict_s = s_m.look_ahead_ms ? (s_m.look_ahead_ms / 1000.f) : s_m.predict;
    s_m.predicted = s_m.smoothed + s_m.velocity * predict_s;
    float perr = s_m.predicted - s_m.smoothed;
    s_m.err_integral = clamp(s_m.err_integral + perr * dt, -100.f, 100.f);
    float ctl = s_m.p_gain * perr + s_m.i_gain * s_m.err_integral;
    int final_d = (int)(s_m.predicted + ctl);
    if (final_d < s_m.min_cm) final_d = s_m.min_cm;
    if (final_d > s_m.max_cm) final_d = s_m.max_cm;
    t->distance_cm = (int16_t)final_d;
    /* PI mode reports raw sensor direction; smoothed velocity sign is
     * noisy near zero, so users get cleaner direction in Kalman mode. */
    if (s_m.velocity > 4.f) t->direction = 1;
    else if (s_m.velocity < -4.f) t->direction = -1;
}

static void run_kalman(float filtered_raw, uint8_t energy, float dt, target_t *t) {
    float vel;
    float pos = kalman_step(&s_m.kf, filtered_raw, energy, dt, &vel);
    float predicted = s_m.look_ahead_ms ? kalman_predict_ahead(&s_m.kf, (float)s_m.look_ahead_ms) : pos;
    if (predicted < s_m.min_cm) predicted = s_m.min_cm;
    if (predicted > s_m.max_cm) predicted = s_m.max_cm;
    t->distance_cm = (int16_t)predicted;
    /* Direction with hysteresis. 4 cm/s threshold: below this we treat the
     * target as stationary (returns 0). */
    t->direction = kalman_direction(&s_m.kf, vel, 4.f);
}

static void motion_task(void *arg) {
    (void)arg;
    radar_frame_t f;
    while (1) {
        if (radar_read(&f, pdMS_TO_TICKS(1000)) != ESP_OK) {
            xSemaphoreTake(s_m.lock, portMAX_DELAY);
            s_m.latest.present = false;
            xSemaphoreGive(s_m.lock);
            continue;
        }

        int raw = f.distance_cm;
        if (raw < s_m.min_cm) raw = s_m.min_cm;
        if (raw > s_m.max_cm) raw = s_m.max_cm;

        /* Outlier rejection (median over W=1/3/7 most recent samples).
         * W=1 effectively bypasses median for users who want it off. */
        s_m.med_buf[s_m.med_idx] = (int16_t)raw;
        s_m.med_idx = (s_m.med_idx + 1) % MEDIAN_MAX_W;
        if (s_m.med_filled < MEDIAN_MAX_W) s_m.med_filled++;
        uint8_t W = outlier_window(s_m.outlier_strength);
        int16_t filtered_raw = median_w(s_m.med_buf, s_m.med_idx, s_m.med_filled, W);

        target_t t = { .present = f.present, .energy = f.energy,
                       .direction = f.direction, .ts_us = f.ts_us };
        t.raw_cm = filtered_raw;

        if (!s_m.enabled) {
            t.distance_cm = filtered_raw;
        } else {
            uint64_t now = f.ts_us;
            float dt = s_m.last_us ? (float)(now - s_m.last_us) / 1e6f : 0.02f;
            s_m.last_us = now;
            dt = clamp(dt, 0.001f, 1.0f);

            if (s_m.mode == MODE_KALMAN) run_kalman((float)filtered_raw, f.energy, dt, &t);
            else                          run_pi   ((float)filtered_raw, dt, &t);
        }

        xSemaphoreTake(s_m.lock, portMAX_DELAY);
        s_m.latest = t;
        xSemaphoreGive(s_m.lock);
    }
}

static void load_config(void) {
    /* Mode: string "kalman" (default) or "pi". */
    char mode[16] = "kalman";
    settings_get_str("motion", "mode", mode, sizeof(mode));
    s_m.mode = (strcmp(mode, "pi") == 0) ? MODE_PI : MODE_KALMAN;

    uint8_t en = 1; settings_get_u8("motion", "en", &en); s_m.enabled = en != 0;

    /* New simplified knobs. */
    uint8_t resp = 50; settings_get_u8("motion", "resp", &resp);
    s_m.response = resp > 100 ? 100 : resp;
    uint32_t la = 0;   settings_get_u32("motion", "la_ms", &la);
    s_m.look_ahead_ms = la > 500 ? 500 : (uint16_t)la;
    uint8_t outl = 1;  settings_get_u8("motion", "outl", &outl);
    s_m.outlier_strength = outl > 2 ? 2 : outl;

    /* Legacy PI knobs. */
    uint32_t v;
    v = (uint32_t)(response_to_pi_alpha(s_m.response) * 1000.f);
    settings_get_u32("motion", "ps", &v); s_m.pos_smooth = v / 1000.f;
    v = 100; settings_get_u32("motion", "vs", &v); s_m.vel_smooth = v / 1000.f;
    v = (uint32_t)(s_m.look_ahead_ms);
    settings_get_u32("motion", "pf", &v); s_m.predict = (v >= 1000) ? (v / 1000.f) : (v / 1000.f);
    v = 100; settings_get_u32("motion", "pg", &v); s_m.p_gain = v / 1000.f;
    v = 10;  settings_get_u32("motion", "ig", &v); s_m.i_gain = v / 1000.f;

    /* Kalman tunables derived from response. R_base 16 cm² (~±4 cm). */
    float Qp, Qv;
    response_to_kalman_q(s_m.response, &Qp, &Qv);
    kalman_set_tunables(&s_m.kf, Qp, Qv, 16.f);

    uint32_t mn = 30, mx = 300;
    settings_get_u32("dist", "min", &mn);
    settings_get_u32("dist", "max", &mx);
    s_m.min_cm = (int)mn; s_m.max_cm = (int)mx;
    if (s_m.max_cm <= s_m.min_cm) { s_m.min_cm = 30; s_m.max_cm = 300; }

    ESP_LOGI(TAG, "Motion: mode=%s en=%d resp=%u la_ms=%u outl=%u  range=%d..%d cm",
             s_m.mode == MODE_KALMAN ? "kalman" : "pi",
             s_m.enabled, s_m.response, s_m.look_ahead_ms, s_m.outlier_strength,
             s_m.min_cm, s_m.max_cm);
    ESP_LOGI(TAG, "Motion (advanced): ps=%.3f vs=%.3f pf=%.3f pg=%.3f ig=%.3f  Qp=%.3f Qv=%.3f",
             s_m.pos_smooth, s_m.vel_smooth, s_m.predict, s_m.p_gain, s_m.i_gain, Qp, Qv);
}

esp_err_t motion_init(void) {
    s_m.lock = xSemaphoreCreateMutex();
    /* Initialize Kalman struct to zero before load_config sets tunables. */
    memset(&s_m.kf, 0, sizeof(s_m.kf));
    load_config();
    xTaskCreate(motion_task, "motion", 4096, NULL, 5, NULL);
    return ESP_OK;
}

void motion_get(target_t *out) {
    if (!out) return;
    xSemaphoreTake(s_m.lock, portMAX_DELAY);
    *out = s_m.latest;
    xSemaphoreGive(s_m.lock);
}

void motion_reload(void) {
    load_config();
    /* Reset filter state so the new mode/tunables take effect cleanly. */
    s_m.kf.initialized = false;
    s_m.smoothed = 0; s_m.predicted = 0; s_m.velocity = 0; s_m.err_integral = 0;
    ESP_LOGI(TAG, "Motion config reloaded");
}
