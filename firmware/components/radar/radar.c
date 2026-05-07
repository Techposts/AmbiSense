#include "radar.h"

#include <string.h>

#include "esp_log.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "settings.h"

static const char *TAG = "radar";

/* Driver function table — each driver implements parse(buf, len) → frame. */
typedef struct {
    const char *id;
    /* Read up to len bytes from UART; on a complete frame, fill out and
     * return number of bytes consumed (>= 1). On partial frame return 0. */
    size_t (*parse)(const uint8_t *buf, size_t len, radar_frame_t *out);
} radar_driver_t;

/* Forward decls — drivers live in radar_<kind>.c */
extern size_t radar_ld2410_parse(const uint8_t *buf, size_t len, radar_frame_t *out);
extern size_t radar_ld2450_parse(const uint8_t *buf, size_t len, radar_frame_t *out);
extern size_t radar_sim_parse   (const uint8_t *buf, size_t len, radar_frame_t *out);

/* v6.2 sensor matrix:
 *   ld2450  — best for stairwell follow-me (x/y target tracking) AND
 *             general presence (auto-falls-back to count-only when
 *             you don't care about position). Kit default.
 *   ld2410c — best for static presence (couch / desk / breathing
 *             detection). Cheaper. Single-target distance-only.
 *   ld2410  — same protocol family as 2410c; legacy support for
 *             v5/v6.0 hardware that already has 2410-class units in
 *             the field.
 *   sim     — synthetic distance trace for desk testing without a
 *             radar wired up.
 */
static const radar_driver_t k_drivers[] = {
    { "ld2450",  radar_ld2450_parse },
    { "ld2410c", radar_ld2410_parse },  /* same protocol family */
    { "ld2410",  radar_ld2410_parse },
    { "sim",     radar_sim_parse    },
};

static struct {
    const radar_driver_t *drv;
    QueueHandle_t q;
    radar_config_t cfg;
    bool inited;
    /* Latest parsed frame, fan-out copy. radar_read() still drives the
     * 1-slot queue (motion drains it once per frame); radar_peek() reads
     * this snapshot under lock so multiple peek consumers (presence,
     * diagnostics, future) can poll without competing with motion. */
    radar_frame_t latest;
    bool          latest_valid;
    SemaphoreHandle_t latest_lock;
    /* Diagnostics — let users debug "distance always 0" by seeing whether
     * UART bytes are arriving and frames are parsing. */
    uint32_t      diag_bytes;
    uint32_t      diag_frames;
    uint64_t      diag_last_frame_us;
    uint8_t       diag_last[64];
    size_t        diag_last_len;
} s_radar;

void radar_get_diag(radar_diag_t *out) {
    if (!out) return;
    memset(out, 0, sizeof(*out));
    if (s_radar.drv) snprintf(out->driver_id, sizeof(out->driver_id), "%s", s_radar.drv->id);
    out->total_bytes_rx = s_radar.diag_bytes;
    out->total_frames_parsed = s_radar.diag_frames;
    if (s_radar.diag_last_frame_us == 0) {
        out->last_frame_age_ms = 0xFFFFFFFFu;
    } else {
        uint64_t now = (uint64_t)esp_timer_get_time();
        out->last_frame_age_ms = (uint32_t)((now - s_radar.diag_last_frame_us) / 1000ULL);
    }
    out->last_bytes_len = s_radar.diag_last_len;
    memcpy(out->last_bytes, s_radar.diag_last, s_radar.diag_last_len);
}

static const radar_driver_t *find_driver(const char *id) {
    for (size_t i = 0; i < sizeof(k_drivers)/sizeof(k_drivers[0]); ++i) {
        if (strcmp(k_drivers[i].id, id) == 0) return &k_drivers[i];
    }
    return NULL;
}

static void radar_task(void *arg) {
    (void)arg;
    static uint8_t rx[512];
    size_t held = 0;
    while (1) {
        if (held >= sizeof(rx)) {
            /* Buffer wedged — discard half to keep parsing forward-progress. */
            memmove(rx, rx + sizeof(rx)/2, sizeof(rx)/2);
            held = sizeof(rx)/2;
        }
        int n = uart_read_bytes(s_radar.cfg.uart_num, rx + held,
                                sizeof(rx) - held, pdMS_TO_TICKS(50));
        if (n > 0) {
            held += n;
            s_radar.diag_bytes += (uint32_t)n;
            /* Keep a rolling window of the last 64 bytes for the hex dump. */
            size_t copy = (size_t)n > sizeof(s_radar.diag_last) ? sizeof(s_radar.diag_last) : (size_t)n;
            if (copy < sizeof(s_radar.diag_last) && s_radar.diag_last_len > 0) {
                size_t shift = sizeof(s_radar.diag_last) - copy;
                size_t keep = s_radar.diag_last_len < shift ? s_radar.diag_last_len : shift;
                memmove(s_radar.diag_last, s_radar.diag_last + (s_radar.diag_last_len - keep), keep);
                s_radar.diag_last_len = keep;
            }
            if (s_radar.diag_last_len + copy > sizeof(s_radar.diag_last)) {
                s_radar.diag_last_len = sizeof(s_radar.diag_last) - copy;
            }
            memcpy(s_radar.diag_last + s_radar.diag_last_len, rx + held - copy, copy);
            s_radar.diag_last_len += copy;
            if (s_radar.diag_last_len > sizeof(s_radar.diag_last))
                s_radar.diag_last_len = sizeof(s_radar.diag_last);
        }

        radar_frame_t frame = {0};
        size_t consumed = s_radar.drv->parse(rx, held, &frame);
        if (consumed > 0) {
            xQueueOverwrite(s_radar.q, &frame);
            if (frame.ts_us != 0) {
                s_radar.diag_frames++;
                s_radar.diag_last_frame_us = frame.ts_us;
            }
            /* Fan-out snapshot for radar_peek() consumers. */
            xSemaphoreTake(s_radar.latest_lock, portMAX_DELAY);
            s_radar.latest = frame;
            s_radar.latest_valid = true;
            xSemaphoreGive(s_radar.latest_lock);
            if (consumed < held) memmove(rx, rx + consumed, held - consumed);
            held -= consumed;
        }
    }
}

esp_err_t radar_init(const radar_config_t *cfg) {
    if (s_radar.inited) return ESP_OK;

    char kind[16] = {0};
    if (settings_get_radar_kind(kind, sizeof(kind)) != ESP_OK || !kind[0]) {
        snprintf(kind, sizeof(kind), "ld2450");  /* user has this on bench */
    }
    s_radar.drv = find_driver(kind);
    if (!s_radar.drv) {
        ESP_LOGE(TAG, "Unknown radar kind '%s'", kind);
        return ESP_ERR_INVALID_ARG;
    }
    ESP_LOGI(TAG, "Radar driver: %s (uart%u, rx=%u, tx=%u, baud=%lu)",
             s_radar.drv->id, cfg->uart_num, cfg->rx_pin, cfg->tx_pin,
             (unsigned long)cfg->baud);

    s_radar.cfg = *cfg;
    s_radar.q = xQueueCreate(1, sizeof(radar_frame_t));
    s_radar.latest_lock = xSemaphoreCreateMutex();

    /* The simulator driver doesn't need UART at all. */
    if (strcmp(s_radar.drv->id, "sim") != 0) {
        const uart_config_t uc = {
            .baud_rate = cfg->baud ? cfg->baud : 256000,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        };
        ESP_ERROR_CHECK(uart_driver_install(cfg->uart_num, 1024, 0, 0, NULL, 0));
        ESP_ERROR_CHECK(uart_param_config(cfg->uart_num, &uc));
        ESP_ERROR_CHECK(uart_set_pin(cfg->uart_num, cfg->tx_pin, cfg->rx_pin,
                                     UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    }

    xTaskCreate(radar_task, "radar", 4096, NULL, 6, NULL);
    s_radar.inited = true;
    return ESP_OK;
}

esp_err_t radar_read(radar_frame_t *out, TickType_t timeout) {
    if (!s_radar.inited) return ESP_ERR_INVALID_STATE;
    if (xQueueReceive(s_radar.q, out, timeout) != pdTRUE) return ESP_ERR_TIMEOUT;
    return ESP_OK;
}

esp_err_t radar_peek(radar_frame_t *out) {
    if (!s_radar.inited || !out) return ESP_ERR_INVALID_STATE;
    if (!s_radar.latest_valid) return ESP_ERR_NOT_FOUND;
    xSemaphoreTake(s_radar.latest_lock, portMAX_DELAY);
    *out = s_radar.latest;
    xSemaphoreGive(s_radar.latest_lock);
    return ESP_OK;
}
