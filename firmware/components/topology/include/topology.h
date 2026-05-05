#pragma once

/*
 * AmbiSense v6 — explicit topology model.
 *
 * Replaces v5's implicit-from-count model that broke asymmetric stairs.
 * Each device knows the full topology: which devices exist, which LED
 * range each owns, and (optionally) which radar distance window each
 * sensor primarily covers.
 *
 * Stored in NVS namespace `topo` as a versioned blob; gossiped between
 * peers via mesh CONFIG packets. The lowest-MAC-numbered device's
 * config wins on ties.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TOPO_MAX_SEGMENTS 8
#define TOPO_MAC_LEN 6

typedef enum {
    TOPO_STRAIGHT = 0,
    TOPO_L_SHAPE  = 1,
    TOPO_U_SHAPE  = 2,
    TOPO_CUSTOM   = 3,
} topology_kind_t;

typedef struct __attribute__((packed)) {
    uint8_t  mac[TOPO_MAC_LEN];   /* device that owns this segment */
    uint16_t led_start;           /* virtual LED address range, inclusive */
    uint16_t led_end;             /* inclusive */
    uint16_t dist_min_cm;         /* distance window this segment covers */
    uint16_t dist_max_cm;         /* (0 = no window, fall through to global) */
    uint8_t  reverse;             /* 1 if strip is physically wired backwards */
    uint8_t  reserved;
} topology_segment_t;

typedef struct __attribute__((packed)) {
    uint16_t version;             /* monotonic; gossip wins higher version */
    uint8_t  kind;                /* topology_kind_t */
    uint8_t  segment_count;
    uint16_t total_leds;          /* virtual address space size */
    uint16_t reserved;
    topology_segment_t segments[TOPO_MAX_SEGMENTS];
} topology_t;

esp_err_t topology_init(void);

/* Return a pointer to the in-RAM topology. Caller must not modify; use
 * topology_set() to update. */
const topology_t *topology_get(void);

/* Replace the topology atomically. If `gossip` is true the mesh layer
 * will broadcast the new version to peers. Persists to NVS. */
esp_err_t topology_set(const topology_t *t, bool gossip);

/* Helper: which segment does the given device own (by MAC), if any? */
const topology_segment_t *topology_segment_for_mac(const uint8_t mac[TOPO_MAC_LEN]);

/* Helper: which segment does this current device own? */
const topology_segment_t *topology_my_segment(void);

#ifdef __cplusplus
}
#endif
