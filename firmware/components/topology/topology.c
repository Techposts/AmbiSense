#include "topology.h"

#include <string.h>

#include "esp_log.h"
#include "esp_mac.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "settings.h"

static const char *TAG = "topology";

static struct {
    topology_t  topo;
    uint8_t     my_mac[TOPO_MAC_LEN];
    SemaphoreHandle_t lock;
    bool        inited;
} s_t;

/* Build a sane default topology for a single-device install: one segment,
 * owned by us, covering the full distance range. */
static void make_default(topology_t *out, const uint8_t mac[TOPO_MAC_LEN]) {
    memset(out, 0, sizeof(*out));
    out->version = 1;
    out->kind = TOPO_STRAIGHT;
    out->segment_count = 1;
    out->total_leds = 30;
    memcpy(out->segments[0].mac, mac, TOPO_MAC_LEN);
    out->segments[0].led_start = 0;
    out->segments[0].led_end   = 29;
    out->segments[0].dist_min_cm = 0;
    out->segments[0].dist_max_cm = 0;  /* 0 = no window */
}

esp_err_t topology_init(void) {
    if (s_t.inited) return ESP_OK;
    s_t.lock = xSemaphoreCreateMutex();
    esp_read_mac(s_t.my_mac, ESP_MAC_WIFI_STA);

    size_t len = sizeof(s_t.topo);
    if (settings_get_blob("topo", "blob", &s_t.topo, &len) != ESP_OK ||
        len != sizeof(s_t.topo) ||
        s_t.topo.segment_count == 0 ||
        s_t.topo.segment_count > TOPO_MAX_SEGMENTS) {
        ESP_LOGI(TAG, "No saved topology; creating default (1 segment, this device)");
        make_default(&s_t.topo, s_t.my_mac);
        settings_set_blob("topo", "blob", &s_t.topo, sizeof(s_t.topo));
    } else {
        ESP_LOGI(TAG, "Loaded topology v%u: kind=%u segments=%u total=%u",
                 s_t.topo.version, s_t.topo.kind, s_t.topo.segment_count, s_t.topo.total_leds);
    }
    s_t.inited = true;
    return ESP_OK;
}

const topology_t *topology_get(void) {
    return &s_t.topo;
}

esp_err_t topology_set(const topology_t *t, bool gossip) {
    if (!t || t->segment_count == 0 || t->segment_count > TOPO_MAX_SEGMENTS) {
        return ESP_ERR_INVALID_ARG;
    }
    xSemaphoreTake(s_t.lock, portMAX_DELAY);
    memcpy(&s_t.topo, t, sizeof(s_t.topo));
    s_t.topo.version++;
    settings_set_blob("topo", "blob", &s_t.topo, sizeof(s_t.topo));
    xSemaphoreGive(s_t.lock);

    ESP_LOGI(TAG, "Topology set: v%u kind=%u segments=%u total=%u (gossip=%d)",
             s_t.topo.version, s_t.topo.kind, s_t.topo.segment_count,
             s_t.topo.total_leds, gossip);
    /* mesh component listens for topology updates and gossips on its own; the
     * `gossip` flag is exposed for callers like mesh_rx that should *not*
     * trigger another gossip when applying a remote update. */
    (void)gossip;
    return ESP_OK;
}

const topology_segment_t *topology_segment_for_mac(const uint8_t mac[TOPO_MAC_LEN]) {
    for (uint8_t i = 0; i < s_t.topo.segment_count; ++i) {
        if (memcmp(s_t.topo.segments[i].mac, mac, TOPO_MAC_LEN) == 0) {
            return &s_t.topo.segments[i];
        }
    }
    return NULL;
}

const topology_segment_t *topology_my_segment(void) {
    return topology_segment_for_mac(s_t.my_mac);
}
