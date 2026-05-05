/*
 * Synthetic radar driver — emits scripted distance traces or a default
 * walk-up/walk-away sine wave.  Lets us iterate on LED modes and mesh
 * fusion without standing in front of a real radar all day.
 *
 * Configure via POST /api/sim/trace with a JSON array of cm distances.
 * Until configured, uses a 4-second cycle 30→200 cm sine wave.
 */

#include <math.h>
#include <string.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "radar.h"

static const char *TAG = "radar_sim";

#define MAX_TRACE 256

static struct {
    int16_t  trace[MAX_TRACE];
    size_t   trace_len;
    uint32_t period_ms;
    int      idx;
    int16_t  last;
} s_sim;

esp_err_t radar_sim_push_trace(const int16_t *distances_cm, size_t n, uint32_t period_ms) {
    if (n > MAX_TRACE) n = MAX_TRACE;
    s_sim.trace_len = n;
    if (n) memcpy(s_sim.trace, distances_cm, n * sizeof(int16_t));
    s_sim.period_ms = period_ms ? period_ms : 100;
    s_sim.idx = 0;
    ESP_LOGI(TAG, "Trace loaded: %u points @ %lu ms", (unsigned)n, (unsigned long)s_sim.period_ms);
    return ESP_OK;
}

/* The simulator never reads from UART; instead, the parser fakes a frame
 * every time it's called. radar_task busy-loops on uart_read_bytes which
 * returns 0 for sim — we add a small delay on each call. */
size_t radar_sim_parse(const uint8_t *buf, size_t len, radar_frame_t *out) {
    (void)buf; (void)len;
    static uint64_t last_us = 0;
    uint64_t now = (uint64_t)esp_timer_get_time();
    uint32_t step_ms = s_sim.period_ms ? s_sim.period_ms : 100;
    if (last_us != 0 && (now - last_us) < (step_ms * 1000ULL)) {
        return 0;
    }
    last_us = now;

    int16_t d;
    if (s_sim.trace_len > 0) {
        d = s_sim.trace[s_sim.idx];
        s_sim.idx = (s_sim.idx + 1) % s_sim.trace_len;
    } else {
        /* Default: 4 s cycle, 30..200 cm. */
        double t = (double)(now / 1000ULL) / 4000.0 * 2.0 * 3.14159265;
        d = (int16_t)(115.0 + 85.0 * sin(t));
    }

    out->present = true;
    out->distance_cm = d;
    out->target_count = 1;
    out->energy = 80;
    out->direction = d < s_sim.last - 2 ? -1 : (d > s_sim.last + 2 ? 1 : 0);
    s_sim.last = d;
    out->targets[0].x_cm = 0;
    out->targets[0].y_cm = d;
    out->targets[0].v_cms = 0;
    out->targets[0].resolution_mm = 100;
    out->ts_us = now;
    return 1;  /* "consumed" any leftover bytes; radar_task moves on. */
}
