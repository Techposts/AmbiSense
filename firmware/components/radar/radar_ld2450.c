/*
 * LD2450 frame parser. 30-byte fixed frames at 256000 baud:
 *
 *   AA FF 03 00                  header
 *   T1: int16 x_mm, int16 y_mm, int16 v_cms, uint16 res_mm   (8 B)
 *   T2: same                                                 (8 B)
 *   T3: same                                                 (8 B)
 *   55 CC                        tail
 *
 * Sign convention is unusual: bit 15 indicates sign, but with the
 * inverse meaning of two's complement.  For each 16-bit field:
 *   if bit15 == 0 → value = -(raw & 0x7FFF)
 *   if bit15 == 1 → value = +(raw & 0x7FFF)
 * So the top bit is "1 for positive, 0 for negative" — opposite of usual.
 *
 * Targets with all fields == 0 are absent. Up to 3 targets per frame.
 */

#include <string.h>
#include "esp_timer.h"

#include "radar.h"

static const uint8_t HEAD[4] = { 0xAA, 0xFF, 0x03, 0x00 };
static const uint8_t TAIL[2] = { 0x55, 0xCC };
#define LD2450_FRAME_LEN 30

static int16_t decode_signed(uint16_t raw) {
    int16_t v = (int16_t)(raw & 0x7FFF);
    return (raw & 0x8000) ? v : -v;
}

size_t radar_ld2450_parse(const uint8_t *buf, size_t len, radar_frame_t *out) {
    for (size_t start = 0; start + LD2450_FRAME_LEN <= len; ++start) {
        if (memcmp(buf + start, HEAD, 4) != 0) continue;
        const uint8_t *t = buf + start + LD2450_FRAME_LEN - 2;
        if (t[0] != TAIL[0] || t[1] != TAIL[1]) {
            return start + 1;  /* false positive on header — skip 1 */
        }
        const uint8_t *p = buf + start + 4;
        uint8_t tcount = 0;
        int16_t primary_distance = 0;
        for (int i = 0; i < RADAR_MAX_TARGETS; ++i) {
            uint16_t xr = p[0] | (p[1] << 8);
            uint16_t yr = p[2] | (p[3] << 8);
            uint16_t vr = p[4] | (p[5] << 8);
            uint16_t rr = p[6] | (p[7] << 8);
            p += 8;
            if (xr == 0 && yr == 0 && vr == 0 && rr == 0) {
                out->targets[i].x_cm = 0;
                out->targets[i].y_cm = 0;
                out->targets[i].v_cms = 0;
                out->targets[i].resolution_mm = 0;
                continue;
            }
            int16_t x_mm = decode_signed(xr);
            int16_t y_mm = decode_signed(yr);
            int16_t v_cs = decode_signed(vr);
            out->targets[i].x_cm = x_mm / 10;
            out->targets[i].y_cm = y_mm / 10;
            out->targets[i].v_cms = v_cs;
            out->targets[i].resolution_mm = rr;
            tcount++;
            if (tcount == 1) {
                /* Primary distance = euclidean from origin in cm. */
                int32_t dx = (int32_t)(x_mm / 10);
                int32_t dy = (int32_t)(y_mm / 10);
                int32_t d2 = dx*dx + dy*dy;
                /* Integer sqrt — fine for radar distance precision. */
                int32_t r = 0; int32_t b = 1L << 14;
                while (b > d2) b >>= 2;
                while (b > 0) {
                    if (d2 >= r + b) { d2 -= r + b; r = (r >> 1) + b; }
                    else r >>= 1;
                    b >>= 2;
                }
                primary_distance = (int16_t)r;
            }
        }
        out->present = tcount > 0;
        out->distance_cm = primary_distance;
        out->target_count = tcount;
        out->energy = 0;  /* LD2450 doesn't report energy; left zero. */

        static int16_t s_last = 0;
        int16_t delta = primary_distance - s_last;
        out->direction = delta < -3 ? -1 : (delta > 3 ? 1 : 0);
        s_last = primary_distance;
        out->ts_us = (uint64_t)esp_timer_get_time();
        return start + LD2450_FRAME_LEN;
    }
    return len > 3 ? len - 3 : 0;
}
