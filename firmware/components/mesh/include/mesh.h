#pragma once

/*
 * AmbiSense v6 — ESP-NOW peer mesh.
 *
 * Each device:
 *   1. Broadcasts its smoothed reading at 5 Hz on the active Wi-Fi channel.
 *   2. Listens for peer broadcasts and runs the same fusion algorithm on
 *      the merged stream — every device arrives at the same active position.
 *   3. Renders only its own LED segment (per topology_my_segment()).
 *
 * Coordinator election: lowest-MAC peer claims the role and serves as the
 * authority for topology gossip and the canonical web UI host advertised
 * via mDNS. Re-runs whenever a peer is lost.
 *
 * Pairing window: 30 s after netmgr boot OR while user holds BOOT button.
 * During the window, broadcast PAIR packets are accepted and added to the
 * peer list. After the window closes, only known peers' packets are
 * accepted (mitigates passive sniffing of neighbors' devices).
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MESH_MAX_PEERS    5    /* including self in some accounting */
#define MESH_MAC_LEN      6
#define MESH_TIMEOUT_MS   10000

typedef enum {
    MESH_FUSE_MOST_RECENT  = 0,
    MESH_FUSE_SLAVE_FIRST  = 1,    /* prefer non-coordinator readings */
    MESH_FUSE_MASTER_FIRST = 2,    /* prefer coordinator's reading */
    MESH_FUSE_ZONE_BASED   = 3,    /* per-segment dist_min/max windows */
} mesh_fusion_t;

typedef struct {
    uint8_t  mac[MESH_MAC_LEN];
    int16_t  distance_cm;
    int8_t   direction;
    uint8_t  energy;
    int8_t   rssi;
    uint64_t last_seen_us;
    bool     healthy;             /* false if last_seen > MESH_TIMEOUT_MS */
} mesh_peer_t;

esp_err_t mesh_init(void);

/* Open a 30 s pairing window during which new peers' broadcasts are
 * accepted. */
esp_err_t mesh_open_pairing(void);

/* Snapshot of currently-known peers (live + stale). Returns count. */
size_t mesh_peers_snapshot(mesh_peer_t *out, size_t max);

/* Fused active-target estimate from local + peer readings. */
typedef struct { int16_t distance_cm; int8_t direction; bool present; uint8_t energy; } mesh_fused_t;
void mesh_get_fused(mesh_fused_t *out);

/* Are we the coordinator (lowest MAC seen)? */
bool mesh_is_coordinator(void);

/* Set fusion policy. Persisted to NVS topo.fuse. */
esp_err_t mesh_set_fusion(mesh_fusion_t mode);
mesh_fusion_t mesh_get_fusion(void);

/* Force a topology gossip after a local change. */
void mesh_gossip_topology(void);

#ifdef __cplusplus
}
#endif
