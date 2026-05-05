#include "mesh.h"

#include <string.h>
#include <inttypes.h>

#include "esp_log.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "settings.h"
#include "motion.h"
#include "topology.h"

static const char *TAG = "mesh";

#define MSG_TARGET     1
#define MSG_HEARTBEAT  2
#define MSG_GOSSIP     3
#define MSG_CHAN_ANN   4
#define MSG_PAIR       5    /* sent every 1 s while pairing window open */
#define MSG_IDENTIFY   6    /* unicast: payload extends with target_mac */

#define MESH_MAGIC 0xA61B

#define PAIRING_WINDOW_MS    30000
#define COORD_HYSTERESIS_US  (5ULL * 1000ULL * 1000ULL)   /* 5 s before role flip */

typedef struct __attribute__((packed)) {
    uint16_t magic;
    uint8_t  msg_type;
    uint8_t  device_idx;
    uint16_t config_version;
    int16_t  distance_cm;
    int8_t   direction;
    uint8_t  energy;
    int16_t  x_cm;
    int16_t  y_cm;
    int16_t  velocity_cms;
    uint8_t  flags;            /* bit0=present, bit1=is_coordinator */
    uint8_t  reserved;
    uint64_t ts_us;
} peer_msg_t;

static const uint8_t BCAST_MAC[6] = { 0xff,0xff,0xff,0xff,0xff,0xff };

static struct {
    uint8_t      my_mac[MESH_MAC_LEN];
    bool         is_coordinator;
    uint64_t     pairing_until_us;
    mesh_peer_t  peers[MESH_MAX_PEERS];
    size_t       peer_count;
    SemaphoreHandle_t lock;
    bool         inited;
    mesh_fusion_t fusion;
    uint16_t     last_gossip_version;

    /* Coordinator hysteresis: when the elected role disagrees with our
     * current role, we wait COORD_HYSTERESIS_US of consistent disagreement
     * before flipping. Prevents flap from a single dropped packet. */
    uint64_t     coord_pending_since_us;

    /* Last time we emitted a MSG_PAIR beacon. */
    uint64_t     last_pair_beacon_us;

    mesh_event_cb_t event_cb;
} s_m;

/* ---- helpers ---- */

static int peer_idx_locked(const uint8_t mac[6]) {
    for (size_t i = 0; i < s_m.peer_count; ++i) {
        if (memcmp(s_m.peers[i].mac, mac, 6) == 0) return (int)i;
    }
    return -1;
}

static bool mac_lt(const uint8_t a[6], const uint8_t b[6]) {
    return memcmp(a, b, 6) < 0;
}

static bool in_pairing_window(void) {
    return (uint64_t)esp_timer_get_time() < s_m.pairing_until_us;
}

static void recompute_coordinator_locked(void) {
    /* Coordinator = lowest-MAC peer that is currently healthy (or self).
     * Hysteresis: once an election outcome differs from our current role
     * we record the time and only flip once the outcome has been stable
     * for COORD_HYSTERESIS_US. This prevents a single dropped heartbeat
     * (which marks a peer "stale" for one tick) from causing two devices
     * to briefly both believe they are the coordinator. */
    uint8_t best[6];
    memcpy(best, s_m.my_mac, 6);
    uint64_t now = (uint64_t)esp_timer_get_time();
    for (size_t i = 0; i < s_m.peer_count; ++i) {
        if (!s_m.peers[i].healthy) continue;
        if (now - s_m.peers[i].last_seen_us > (uint64_t)MESH_TIMEOUT_MS * 1000ULL) continue;
        if (mac_lt(s_m.peers[i].mac, best)) memcpy(best, s_m.peers[i].mac, 6);
    }
    bool desired = (memcmp(best, s_m.my_mac, 6) == 0);

    if (desired == s_m.is_coordinator) {
        s_m.coord_pending_since_us = 0;
        return;
    }
    if (s_m.coord_pending_since_us == 0) {
        s_m.coord_pending_since_us = now;
        ESP_LOGI(TAG, "Coordinator change pending → %s (5 s hysteresis)",
                 desired ? "this device" : "peer");
        return;
    }
    if (now - s_m.coord_pending_since_us >= COORD_HYSTERESIS_US) {
        s_m.is_coordinator = desired;
        s_m.coord_pending_since_us = 0;
        ESP_LOGI(TAG, "Coordinator role: %s", s_m.is_coordinator ? "this device" : "peer");
    }
}

static void add_peer_locked(const uint8_t mac[6]) {
    if (peer_idx_locked(mac) >= 0) return;
    if (s_m.peer_count >= MESH_MAX_PEERS) {
        ESP_LOGW(TAG, "Peer table full; rejecting new peer");
        return;
    }
    /* Add to esp_now peer list as well so unicast works. */
    esp_now_peer_info_t info = {0};
    memcpy(info.peer_addr, mac, 6);
    info.channel = 0;
    info.ifidx = WIFI_IF_AP;     /* mirror our broadcast channel via AP */
    info.encrypt = false;
    esp_now_add_peer(&info);

    memcpy(s_m.peers[s_m.peer_count].mac, mac, 6);
    s_m.peers[s_m.peer_count].last_seen_us = (uint64_t)esp_timer_get_time();
    s_m.peers[s_m.peer_count].healthy = true;
    s_m.peer_count++;
    ESP_LOGI(TAG, "Peer added: %02x:%02x:%02x:%02x:%02x:%02x  (count=%u)",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], (unsigned)s_m.peer_count);

    /* Notify upper layer outside the lock would be cleaner, but our cb is
     * documented as fast-callable and the lock is uncontended at this
     * point — fire it inline. */
    if (s_m.event_cb) s_m.event_cb(MESH_EVT_PEER_JOINED, mac);
}

/* ---- ESP-NOW recv callback ---- */

static void on_recv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len < (int)sizeof(peer_msg_t)) return;
    const peer_msg_t *m = (const peer_msg_t *)data;
    if (m->magic != MESH_MAGIC) return;
    if (memcmp(info->src_addr, s_m.my_mac, 6) == 0) return;  /* our own */

    /* IDENTIFY is special: handle outside the lock so we can fire the
     * status-LED reaction without holding the mesh mutex while a status
     * LED API runs. We still snapshot whether we're the target. */
    bool identify_for_me = false;
    if (m->msg_type == MSG_IDENTIFY && len >= (int)sizeof(peer_msg_t) + 6) {
        const uint8_t *target = data + sizeof(peer_msg_t);
        identify_for_me = (memcmp(target, s_m.my_mac, 6) == 0);
    }

    /* Asymmetric pairing: receiving a MSG_PAIR from anyone reopens our
     * own window. So clicking "Pair" on either device pulls the other in
     * — no need to tap both physical buttons. */
    bool opened_window = false;
    if (m->msg_type == MSG_PAIR && !in_pairing_window()) {
        s_m.pairing_until_us = (uint64_t)esp_timer_get_time() + (uint64_t)PAIRING_WINDOW_MS * 1000ULL;
        opened_window = true;
        ESP_LOGI(TAG, "Pairing auto-opened by peer beacon");
    }

    xSemaphoreTake(s_m.lock, portMAX_DELAY);
    int idx = peer_idx_locked(info->src_addr);
    if (idx < 0) {
        if (in_pairing_window() || m->msg_type == MSG_PAIR) {
            add_peer_locked(info->src_addr);
            idx = peer_idx_locked(info->src_addr);
        } else {
            xSemaphoreGive(s_m.lock);
            return;
        }
    }
    if (idx >= 0) {
        s_m.peers[idx].distance_cm = m->distance_cm;
        s_m.peers[idx].direction   = m->direction;
        s_m.peers[idx].energy      = m->energy;
        s_m.peers[idx].rssi        = info->rx_ctrl ? info->rx_ctrl->rssi : 0;
        s_m.peers[idx].last_seen_us = (uint64_t)esp_timer_get_time();
        s_m.peers[idx].healthy = true;
    }

    if (m->msg_type == MSG_GOSSIP) {
        /* Topology gossip: payload extends past peer_msg_t with a topology_t. */
        if (len >= (int)(sizeof(peer_msg_t) + sizeof(topology_t))) {
            const topology_t *remote = (const topology_t *)(data + sizeof(peer_msg_t));
            const topology_t *local  = topology_get();
            if (remote->version > local->version) {
                ESP_LOGI(TAG, "Accepting gossiped topology v%u (was v%u)",
                         remote->version, local->version);
                topology_set(remote, false);  /* don't re-gossip */
            }
        }
    }

    recompute_coordinator_locked();
    xSemaphoreGive(s_m.lock);

    /* Fire deferred events outside the lock. */
    if (opened_window && s_m.event_cb) s_m.event_cb(MESH_EVT_PAIRING_OPENED, s_m.my_mac);
    if (identify_for_me && s_m.event_cb) s_m.event_cb(MESH_EVT_IDENTIFY_REQUESTED, info->src_addr);
}

/* ---- broadcast task: 5 Hz target broadcast, 1 Hz pair beacon ---- */

static void make_msg(peer_msg_t *out, uint8_t type) {
    target_t t;
    motion_get(&t);
    memset(out, 0, sizeof(*out));
    out->magic = MESH_MAGIC;
    out->msg_type = type;
    out->config_version = topology_get()->version;
    out->distance_cm = t.distance_cm;
    out->direction = t.direction;
    out->energy = t.energy;
    out->flags = (t.present ? 1 : 0) | (s_m.is_coordinator ? 2 : 0);
    out->ts_us = (uint64_t)esp_timer_get_time();
}

static void broadcast_task(void *arg) {
    (void)arg;
    bool was_pairing = false;
    while (1) {
        peer_msg_t msg;
        make_msg(&msg, MSG_TARGET);
        esp_now_send(BCAST_MAC, (const uint8_t *)&msg, sizeof(msg));

        /* MSG_PAIR beacon at 1 Hz while window open. Tracks state edges so
         * the event callback is fired exactly once on open and on close. */
        bool pairing_now = in_pairing_window();
        if (pairing_now) {
            uint64_t now = (uint64_t)esp_timer_get_time();
            if (now - s_m.last_pair_beacon_us >= 1000000ULL) {
                peer_msg_t pm;
                make_msg(&pm, MSG_PAIR);
                esp_now_send(BCAST_MAC, (const uint8_t *)&pm, sizeof(pm));
                s_m.last_pair_beacon_us = now;
            }
        }
        if (was_pairing && !pairing_now) {
            ESP_LOGI(TAG, "Pairing window closed");
            if (s_m.event_cb) s_m.event_cb(MESH_EVT_PAIRING_CLOSED, s_m.my_mac);
        }
        was_pairing = pairing_now;

        /* Mark stale peers + recompute coordinator + step to next tick. */
        xSemaphoreTake(s_m.lock, portMAX_DELAY);
        uint64_t now = (uint64_t)esp_timer_get_time();
        for (size_t i = 0; i < s_m.peer_count; ++i) {
            uint64_t age = now - s_m.peers[i].last_seen_us;
            bool was_h = s_m.peers[i].healthy;
            s_m.peers[i].healthy = age < ((uint64_t)MESH_TIMEOUT_MS * 1000ULL);
            if (was_h != s_m.peers[i].healthy) {
                ESP_LOGW(TAG, "Peer %02x:%02x:%02x:%02x:%02x:%02x is now %s",
                    s_m.peers[i].mac[0], s_m.peers[i].mac[1], s_m.peers[i].mac[2],
                    s_m.peers[i].mac[3], s_m.peers[i].mac[4], s_m.peers[i].mac[5],
                    s_m.peers[i].healthy ? "healthy" : "STALE");
            }
        }
        recompute_coordinator_locked();
        xSemaphoreGive(s_m.lock);

        vTaskDelay(pdMS_TO_TICKS(200));  /* 5 Hz */
    }
}

/* ---- public API ---- */

esp_err_t mesh_init(void) {
    if (s_m.inited) return ESP_OK;
    s_m.lock = xSemaphoreCreateMutex();
    esp_read_mac(s_m.my_mac, ESP_MAC_WIFI_STA);

    /* ESP-NOW must be initialized after Wi-Fi is started. */
    esp_err_t err = esp_now_init();
    if (err != ESP_OK) { ESP_LOGE(TAG, "esp_now_init: 0x%x", err); return err; }

    /* Broadcast peer is implicit on most IDF versions; explicit add for safety. */
    esp_now_peer_info_t bcast = {0};
    memcpy(bcast.peer_addr, BCAST_MAC, 6);
    bcast.channel = 0;     /* current */
    bcast.ifidx = WIFI_IF_AP;
    bcast.encrypt = false;
    esp_now_add_peer(&bcast);

    esp_now_register_recv_cb(on_recv);

    /* Restore fusion mode from NVS. */
    uint8_t f = MESH_FUSE_MOST_RECENT;
    settings_get_u8("topo", "fuse", &f);
    s_m.fusion = (mesh_fusion_t)f;

    /* Open a pairing window at boot — first 30 s lets uninitialized peers
     * find each other without manual button presses. */
    s_m.pairing_until_us = (uint64_t)esp_timer_get_time() + (uint64_t)PAIRING_WINDOW_MS * 1000ULL;
    s_m.is_coordinator = true;  /* Until proven otherwise by a lower-MAC peer. */

    xTaskCreate(broadcast_task, "mesh_bcast", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Mesh up. MAC %02x:%02x:%02x:%02x:%02x:%02x; pairing window 30 s",
             s_m.my_mac[0], s_m.my_mac[1], s_m.my_mac[2],
             s_m.my_mac[3], s_m.my_mac[4], s_m.my_mac[5]);
    s_m.inited = true;
    return ESP_OK;
}

esp_err_t mesh_open_pairing(void) {
    s_m.pairing_until_us = (uint64_t)esp_timer_get_time() + (uint64_t)PAIRING_WINDOW_MS * 1000ULL;
    s_m.last_pair_beacon_us = 0;   /* force immediate beacon on next tick */
    ESP_LOGI(TAG, "Pairing window opened (30 s)");
    if (s_m.event_cb) s_m.event_cb(MESH_EVT_PAIRING_OPENED, s_m.my_mac);
    return ESP_OK;
}

bool mesh_in_pairing(void) {
    return in_pairing_window();
}

uint32_t mesh_pairing_remaining_ms(void) {
    uint64_t now = (uint64_t)esp_timer_get_time();
    if (now >= s_m.pairing_until_us) return 0;
    return (uint32_t)((s_m.pairing_until_us - now) / 1000ULL);
}

esp_err_t mesh_identify(const uint8_t mac[6]) {
    if (!mac) return ESP_ERR_INVALID_ARG;
    /* Add the peer to esp-now if we don't have it yet — the user may be
     * identifying a topology-listed device that isn't currently a peer. */
    xSemaphoreTake(s_m.lock, portMAX_DELAY);
    if (peer_idx_locked(mac) < 0) add_peer_locked(mac);
    xSemaphoreGive(s_m.lock);

    uint8_t buf[sizeof(peer_msg_t) + 6];
    peer_msg_t *m = (peer_msg_t *)buf;
    make_msg(m, MSG_IDENTIFY);
    memcpy(buf + sizeof(peer_msg_t), mac, 6);
    esp_err_t err = esp_now_send(mac, buf, sizeof(buf));
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "mesh_identify: esp_now_send failed 0x%x", err);
    } else {
        ESP_LOGI(TAG, "Identify request sent to %02x:%02x:%02x:%02x:%02x:%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
    return err;
}

void mesh_set_event_cb(mesh_event_cb_t cb) {
    s_m.event_cb = cb;
}

size_t mesh_peers_snapshot(mesh_peer_t *out, size_t max) {
    xSemaphoreTake(s_m.lock, portMAX_DELAY);
    size_t n = s_m.peer_count < max ? s_m.peer_count : max;
    memcpy(out, s_m.peers, n * sizeof(mesh_peer_t));
    xSemaphoreGive(s_m.lock);
    return n;
}

void mesh_get_fused(mesh_fused_t *out) {
    if (!out) return;
    target_t local;
    motion_get(&local);

    xSemaphoreTake(s_m.lock, portMAX_DELAY);
    uint64_t now = (uint64_t)esp_timer_get_time();

    /* Build a list of candidate readings: local + healthy peers. */
    struct { int16_t d; int8_t dir; uint8_t en; bool from_coord; uint64_t ts; } cand[MESH_MAX_PEERS + 1];
    size_t nc = 0;
    cand[nc++] = (typeof(cand[0])){ local.distance_cm, local.direction, local.energy, s_m.is_coordinator, local.ts_us };
    for (size_t i = 0; i < s_m.peer_count; ++i) {
        if (!s_m.peers[i].healthy) continue;
        bool peer_is_coord = mac_lt(s_m.peers[i].mac, s_m.my_mac);  /* lower-MAC than us */
        cand[nc++] = (typeof(cand[0])){ s_m.peers[i].distance_cm, s_m.peers[i].direction,
                                        s_m.peers[i].energy, peer_is_coord, s_m.peers[i].last_seen_us };
        if (nc >= sizeof(cand)/sizeof(cand[0])) break;
    }

    int chosen = 0;
    switch (s_m.fusion) {
        case MESH_FUSE_MOST_RECENT: {
            uint64_t newest = 0;
            for (size_t i = 0; i < nc; ++i) if (cand[i].ts > newest) { newest = cand[i].ts; chosen = (int)i; }
            break;
        }
        case MESH_FUSE_SLAVE_FIRST: {
            int slave_pick = -1; uint64_t slave_t = 0;
            for (size_t i = 0; i < nc; ++i) if (!cand[i].from_coord && cand[i].en > 0 && cand[i].ts > slave_t) { slave_t = cand[i].ts; slave_pick = (int)i; }
            if (slave_pick >= 0) chosen = slave_pick;
            else { uint64_t newest = 0; for (size_t i = 0; i < nc; ++i) if (cand[i].ts > newest) { newest = cand[i].ts; chosen = (int)i; } }
            break;
        }
        case MESH_FUSE_MASTER_FIRST: {
            int master_pick = -1; uint64_t master_t = 0;
            for (size_t i = 0; i < nc; ++i) if (cand[i].from_coord && cand[i].en > 0 && cand[i].ts > master_t) { master_t = cand[i].ts; master_pick = (int)i; }
            if (master_pick >= 0) chosen = master_pick;
            else { uint64_t newest = 0; for (size_t i = 0; i < nc; ++i) if (cand[i].ts > newest) { newest = cand[i].ts; chosen = (int)i; } }
            break;
        }
        case MESH_FUSE_ZONE_BASED: {
            /* Each candidate is rated by which segment's distance window it
             * lands in; the candidate matching its segment's range wins. */
            int best = 0; int best_score = -1;
            const topology_t *t = topology_get();
            for (size_t i = 0; i < nc; ++i) {
                int score = (int)cand[i].en;  /* baseline */
                for (uint8_t s = 0; s < t->segment_count; ++s) {
                    if (t->segments[s].dist_max_cm == 0) continue;  /* no window */
                    if (cand[i].d >= t->segments[s].dist_min_cm &&
                        cand[i].d <= t->segments[s].dist_max_cm) {
                        score += 100;
                    }
                }
                if (score > best_score) { best_score = score; best = (int)i; }
            }
            chosen = best;
            break;
        }
    }

    out->distance_cm = cand[chosen].d;
    out->direction   = cand[chosen].dir;
    out->energy      = cand[chosen].en;
    out->present     = cand[chosen].en > 0;
    (void)now;
    xSemaphoreGive(s_m.lock);
}

bool mesh_is_coordinator(void) { return s_m.is_coordinator; }

esp_err_t mesh_set_fusion(mesh_fusion_t mode) {
    if (mode > MESH_FUSE_ZONE_BASED) return ESP_ERR_INVALID_ARG;
    s_m.fusion = mode;
    settings_set_u8("topo", "fuse", (uint8_t)mode);
    ESP_LOGI(TAG, "Fusion mode set to %d", mode);
    return ESP_OK;
}

mesh_fusion_t mesh_get_fusion(void) { return s_m.fusion; }

void mesh_gossip_topology(void) {
    /* peer_msg_t header + topology_t blob */
    uint8_t buf[sizeof(peer_msg_t) + sizeof(topology_t)];
    peer_msg_t *m = (peer_msg_t *)buf;
    make_msg(m, MSG_GOSSIP);
    memcpy(buf + sizeof(peer_msg_t), topology_get(), sizeof(topology_t));
    esp_now_send(BCAST_MAC, buf, sizeof(buf));
    ESP_LOGI(TAG, "Gossiped topology v%u", topology_get()->version);
}
