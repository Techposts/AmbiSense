/*
 * LD2410 / LD2412 / LD2420 frame parser.
 *
 * Frame layout (data report, "basic" mode, 23 bytes):
 *   F4 F3 F2 F1                  header
 *   <length:2 LE>                payload length (always 0x000D for basic)
 *   02                           data type: target data
 *   AA                           head
 *   <state:1>                    0=none, 1=moving, 2=stationary, 3=both
 *   <moving_distance:2 LE>       cm
 *   <moving_energy:1>            0..100
 *   <stationary_distance:2 LE>   cm
 *   <stationary_energy:1>        0..100
 *   <detection_distance:2 LE>    cm
 *   55                           tail
 *   00                           checksum-ish
 *   F8 F7 F6 F5                  trailer
 *
 * The LD2412 / LD2420 stream identical layout for the basic data report.
 * If "engineering mode" is enabled the frame is longer; we just skip it.
 */

#include <string.h>
#include "esp_timer.h"

#include "radar.h"

static const uint8_t HEAD[4] = { 0xF4, 0xF3, 0xF2, 0xF1 };
static const uint8_t TAIL[4] = { 0xF8, 0xF7, 0xF6, 0xF5 };

size_t radar_ld2410_parse(const uint8_t *buf, size_t len, radar_frame_t *out) {
    /* Find header */
    for (size_t start = 0; start + 8 < len; ++start) {
        if (memcmp(buf + start, HEAD, 4) != 0) continue;
        if (start + 6 > len) return 0;  /* need length bytes */
        uint16_t plen = buf[start + 4] | (buf[start + 5] << 8);
        size_t total = 4 /*head*/ + 2 /*len*/ + plen + 4 /*tail*/;
        if (start + total > len) return 0;  /* incomplete */
        const uint8_t *p = buf + start + 6;
        const uint8_t *tail = buf + start + 6 + plen;
        if (memcmp(tail, TAIL, 4) != 0) {
            /* Header without matching tail in expected position; advance one byte
             * past header and try again on next loop iteration. */
            return start + 1;
        }
        /* Validate the basic-mode payload (type 02, head AA, body len 13). */
        if (plen >= 13 && p[0] == 0x02 && p[1] == 0xAA) {
            uint8_t state = p[2];
            int16_t mov_dist = (int16_t)(p[3] | (p[4] << 8));
            uint8_t mov_e    = p[5];
            int16_t sta_dist = (int16_t)(p[6] | (p[7] << 8));
            uint8_t sta_e    = p[8];

            int16_t dist = 0;
            uint8_t energy = 0;
            if (state & 0x01) {            /* moving target */
                dist = mov_dist;
                energy = mov_e;
            } else if (state & 0x02) {     /* stationary fallback */
                dist = sta_dist;
                energy = sta_e;
            }

            out->present = (state != 0) && dist > 0;
            out->distance_cm = dist;
            out->energy = energy;
            out->target_count = out->present ? 1 : 0;

            /* Naive direction = sign of distance change vs last frame. */
            static int16_t s_last = 0;
            int16_t delta = dist - s_last;
            out->direction = delta < -3 ? -1 : (delta > 3 ? 1 : 0);
            s_last = dist;

            out->ts_us = (uint64_t)esp_timer_get_time();
        }
        return start + total;
    }
    /* No header in buffer (or only at very end) — keep last 3 bytes. */
    return len > 3 ? len - 3 : 0;
}
