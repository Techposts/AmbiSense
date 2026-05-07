#pragma once

/*
 * AmbiSense v6 — radar abstraction.
 *
 * One unified frame type, one registry, one driver instantiated at boot
 * based on the NVS `board.radar_kind` key. Drivers are linked into the
 * binary unconditionally; selection is runtime so users can swap sensors
 * via the web UI without reflashing.
 *
 * v6.x drivers (single-sensor architecture, LD2450 only on shipping
 * hardware; sim retained for desk testing without a radar attached):
 *   ld2450   — HiLink LD2450, up to 3 targets with x/y/speed (24 GHz)
 *   sim      — synthetic trace generator for desk testing
 */

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RADAR_MAX_TARGETS 3

typedef struct {
    bool      present;             /* primary target present */
    int16_t   distance_cm;         /* primary target distance, 0 if absent */
    int8_t    direction;           /* -1 closer, 0 still, +1 away */
    uint8_t   energy;              /* signal strength 0..100 (where exposed) */
    uint8_t   target_count;        /* 0..3 (LD2450 reports up to 3 simultaneous targets) */
    struct {
        int16_t  x_cm;             /* lateral; left negative, right positive */
        int16_t  y_cm;             /* radial */
        int16_t  v_cms;            /* speed cm/s; +ve = away from sensor */
        uint16_t resolution_mm;    /* LD2450 reports per-target resolution */
    } targets[RADAR_MAX_TARGETS];
    uint64_t  ts_us;
} radar_frame_t;

typedef struct {
    uint8_t  uart_num;
    uint8_t  rx_pin;
    uint8_t  tx_pin;
    uint32_t baud;                 /* 256000 for LD-family */
} radar_config_t;

/* Set up the active driver from NVS-selected kind. Spawns a task that
 * continuously parses radar frames and pushes them to an internal queue. */
esp_err_t radar_init(const radar_config_t *cfg);

/* Block until a frame is available or the timeout expires. */
esp_err_t radar_read(radar_frame_t *out, TickType_t timeout);

/* For the simulator driver — replay a scripted trace. */
esp_err_t radar_sim_push_trace(const int16_t *distances_cm, size_t n, uint32_t period_ms);

/* Diagnostics: helps debug "distance is always 0" — tells you whether
 * UART bytes are even arriving from the radar. */
typedef struct {
    char     driver_id[16];
    uint32_t total_bytes_rx;       /* bytes read from UART since boot */
    uint32_t total_frames_parsed;  /* successfully parsed radar frames */
    uint32_t last_frame_age_ms;    /* 0 if never; UINT32_MAX if stale */
    uint8_t  last_bytes[64];       /* most recent raw bytes for hex dump */
    size_t   last_bytes_len;
} radar_diag_t;

void radar_get_diag(radar_diag_t *out);

#ifdef __cplusplus
}
#endif
