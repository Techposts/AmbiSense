#include "led_engine.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "motion.h"
#include "settings.h"

static const char *TAG = "led_engine";

#define MAX_LEDS 1500
#define DEFAULT_LEDS 30
#define MAX_PARTICLES 50

typedef struct { uint8_t r, g, b; } rgb_t;
typedef struct { float pos; float vel; float bright; bool active; } particle_t;

static struct {
    led_strip_handle_t strip;
    uint8_t   data_gpio;
    uint16_t  count;
    uint8_t   brightness;
    rgb_t     base;
    led_mode_t mode;
    uint16_t  span;
    int16_t   center_shift;
    uint8_t   trail;
    bool      dir_light;
    bool      bg_mode;
    uint8_t   effect_speed;
    uint8_t   effect_intensity;
    uint16_t  min_cm, max_cm;

    /* Mode-private state */
    uint32_t  step;            /* effect frame counter */
    uint8_t  *fire_heat;
    particle_t particles[MAX_PARTICLES];
    rgb_t    *prev_frame;      /* for COMET tail fade */
} s_led;

/* ------ utilities -------------------------------------------------- */

static rgb_t dim(rgb_t c, float k) {
    return (rgb_t){(uint8_t)(c.r * k), (uint8_t)(c.g * k), (uint8_t)(c.b * k)};
}

static rgb_t wheel(uint8_t pos) {
    /* 0..255 → rainbow (port of v5 wheelColor) */
    if (pos < 85)  return (rgb_t){ pos*3,         255 - pos*3,  0 };
    if (pos < 170) { pos -= 85;  return (rgb_t){ 255 - pos*3, 0,           pos*3 }; }
                   { pos -= 170; return (rgb_t){ 0,           pos*3,       255 - pos*3 }; }
}

static int distance_to_start_led(int distance_cm) {
    if (s_led.max_cm <= s_led.min_cm) return 0;
    int range = s_led.max_cm - s_led.min_cm;
    int span_pixels = s_led.span > 0 ? s_led.span : 30;
    int avail = (int)s_led.count - span_pixels;
    if (avail < 0) avail = 0;
    int rel = distance_cm - s_led.min_cm;
    if (rel < 0) rel = 0;
    if (rel > range) rel = range;
    int p = (rel * avail) / range;
    p += s_led.center_shift;
    if (p < 0) p = 0;
    if (p > avail) p = avail;
    return p;
}

static void clear_all(void) {
    led_strip_clear(s_led.strip);
}

static inline void set_pixel(int i, rgb_t c) {
    if (i < 0 || i >= (int)s_led.count) return;
    /* Apply global brightness scaling here so modes don't need to. */
    float k = s_led.brightness / 255.0f;
    led_strip_set_pixel(s_led.strip, i, (uint8_t)(c.r * k),
                                        (uint8_t)(c.g * k),
                                        (uint8_t)(c.b * k));
}

/* ------ modes ------------------------------------------------------ */

static void mode_standard(int start) {
    int span = s_led.span > 0 ? s_led.span : 30;
    if (s_led.bg_mode) {
        rgb_t bg = dim(s_led.base, 0.05f);
        for (int i = 0; i < s_led.count; ++i) set_pixel(i, bg);
    } else {
        clear_all();
    }
    for (int i = start; i < start + span && i < s_led.count; ++i) {
        set_pixel(i, s_led.base);
    }
    if (s_led.dir_light && s_led.trail > 0) {
        int trail = s_led.trail;
        for (int i = 0; i < trail && (start - 1 - i) >= 0; ++i) {
            float k = 1.0f - (float)(i + 1) / (trail + 1);
            set_pixel(start - 1 - i, dim(s_led.base, k));
        }
    }
}

static void mode_rainbow(void) {
    uint8_t step = (uint8_t)(s_led.step * (s_led.effect_speed > 0 ? s_led.effect_speed/12 : 1));
    for (int i = 0; i < s_led.count; ++i) {
        set_pixel(i, wheel((uint8_t)((i * 256 / s_led.count + step) & 0xFF)));
    }
}

static void mode_color_wave(void) {
    float t = s_led.step * 0.05f * (s_led.effect_speed/50.0f + 0.5f);
    float intensity = s_led.effect_intensity / 100.0f;
    for (int i = 0; i < s_led.count; ++i) {
        float phase = (float)i / s_led.count * 6.28318f + t;
        float k = (sinf(phase) + 1.0f) * 0.5f;
        k = k * intensity + (1.0f - intensity) * 0.4f;
        rgb_t c = wheel((uint8_t)((i * 256 / s_led.count + s_led.step) & 0xFF));
        set_pixel(i, dim(c, k));
    }
}

static void mode_breathing(void) {
    float speed = s_led.effect_speed / 50.0f;
    float k = (sinf(s_led.step * 0.05f * speed) + 1.0f) * 0.5f;
    k = 0.1f + 0.9f * k * (s_led.effect_intensity / 100.0f);
    for (int i = 0; i < s_led.count; ++i) set_pixel(i, dim(s_led.base, k));
}

static void mode_solid(void) {
    for (int i = 0; i < s_led.count; ++i) set_pixel(i, s_led.base);
}

static void mode_comet(int start) {
    if (!s_led.prev_frame) return;
    /* Fade entire previous frame */
    float fade = 0.85f - (s_led.effect_speed / 1000.0f);
    if (fade < 0.7f) fade = 0.7f;
    for (int i = 0; i < s_led.count; ++i) {
        s_led.prev_frame[i] = dim(s_led.prev_frame[i], fade);
        set_pixel(i, s_led.prev_frame[i]);
    }
    /* Bright head at start */
    int head_w = 3;
    for (int i = 0; i < head_w; ++i) {
        int p = start + i;
        if (p >= 0 && p < s_led.count) {
            s_led.prev_frame[p] = s_led.base;
            set_pixel(p, s_led.base);
        }
    }
}

static void mode_pulse(int start) {
    clear_all();
    float intensity = s_led.effect_intensity / 100.0f;
    int max_radius = s_led.count / 4;
    for (int p = 0; p < 3; ++p) {
        float phase = (float)p * 2.0f;
        float r = fmodf(s_led.step * 0.2f + phase, max_radius);
        for (int off = -(int)r; off <= (int)r; ++off) {
            float dist = fabsf(off / r);
            float k = (1.0f - dist * dist) * intensity;
            int idx = start + off;
            if (idx >= 0 && idx < s_led.count) {
                rgb_t c = dim(s_led.base, k);
                set_pixel(idx, c);
            }
        }
    }
}

static uint8_t qadd8(uint8_t a, uint8_t b) { unsigned s = a + b; return s > 255 ? 255 : s; }
static uint8_t qsub8(uint8_t a, uint8_t b) { return a > b ? a - b : 0; }

static void mode_fire(void) {
    if (!s_led.fire_heat) return;
    int n = s_led.count;
    /* Cool */
    int cooling = 55;
    for (int i = 0; i < n; ++i) {
        s_led.fire_heat[i] = qsub8(s_led.fire_heat[i], (uint8_t)((rand() % cooling) + 2));
    }
    /* Drift up */
    for (int k = n - 1; k >= 2; --k) {
        s_led.fire_heat[k] = (uint8_t)((s_led.fire_heat[k-1] + s_led.fire_heat[k-2] + s_led.fire_heat[k-2]) / 3);
    }
    /* Sparks */
    int sparking = 120;
    if ((rand() & 0xFF) < sparking) {
        int y = rand() % 7;
        s_led.fire_heat[y] = qadd8(s_led.fire_heat[y], (uint8_t)(160 + (rand() % 96)));
    }
    /* Render */
    for (int i = 0; i < n; ++i) {
        uint8_t t = (uint8_t)((s_led.fire_heat[i] * 191) / 255);
        rgb_t c;
        if (t < 64)        c = (rgb_t){ t * 4,           0,                  0 };
        else if (t < 128)  c = (rgb_t){ 255,            (t - 64) * 4,        0 };
        else               c = (rgb_t){ 255,            255,                 (t - 128) * 4 };
        set_pixel(i, c);
    }
}

static void mode_theater_chase(void) {
    int gap = 3;
    int phase = s_led.step % gap;
    for (int i = 0; i < s_led.count; ++i) {
        if ((i + phase) % gap == 0) set_pixel(i, s_led.base);
        else                        set_pixel(i, (rgb_t){0,0,0});
    }
}

static void mode_dual_scan(int start) {
    clear_all();
    int scan_w = 4;
    int p1 = (s_led.step) % s_led.count;
    int p2 = (s_led.count - 1 - (s_led.step % s_led.count));
    for (int i = -scan_w; i <= scan_w; ++i) {
        float k = 1.0f - (float)abs(i) / scan_w;
        if (p1 + i >= 0 && p1 + i < s_led.count) set_pixel(p1 + i, dim(s_led.base, k));
        if (p2 + i >= 0 && p2 + i < s_led.count) set_pixel(p2 + i, dim((rgb_t){255 - s_led.base.r, 255 - s_led.base.g, 255 - s_led.base.b}, k));
    }
    /* Brighter point at the active distance */
    if (start >= 0 && start < s_led.count) set_pixel(start, (rgb_t){255, 255, 255});
}

static void mode_motion_particles(int start) {
    /* Decay all */
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (!s_led.particles[i].active) continue;
        s_led.particles[i].pos += s_led.particles[i].vel;
        s_led.particles[i].bright -= 0.02f;
        if (s_led.particles[i].bright <= 0 ||
            s_led.particles[i].pos < 0 ||
            s_led.particles[i].pos >= s_led.count) {
            s_led.particles[i].active = false;
        }
    }
    /* Spawn new particles around `start`. */
    int to_spawn = 1 + (s_led.effect_intensity / 25);
    for (int n = 0; n < to_spawn; ++n) {
        for (int i = 0; i < MAX_PARTICLES; ++i) {
            if (s_led.particles[i].active) continue;
            s_led.particles[i].active = true;
            s_led.particles[i].pos = (float)start + (rand() % 7) - 3;
            s_led.particles[i].vel = ((rand() % 200) - 100) / 100.0f * (s_led.effect_speed/100.0f + 0.3f);
            s_led.particles[i].bright = 1.0f;
            break;
        }
    }
    /* Render */
    clear_all();
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (!s_led.particles[i].active) continue;
        int p = (int)s_led.particles[i].pos;
        if (p >= 0 && p < s_led.count) {
            set_pixel(p, dim(s_led.base, s_led.particles[i].bright));
        }
    }
}

/* ------ main render task ------------------------------------------- */

static void render_task(void *arg) {
    (void)arg;
    while (1) {
        /* Honor the global "system enabled" flag — when off, paint black
         * and refresh once per frame rather than skipping (so a disabled
         * strip stays dark even if FreeRTOS switches in mid-frame). */
        uint8_t sys_en = 1;
        settings_get_u8("sys", "enabled", &sys_en);
        if (!sys_en) {
            for (int i = 0; i < s_led.count; ++i) set_pixel(i, (rgb_t){0,0,0});
            led_strip_refresh(s_led.strip);
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        target_t t = {0};
        motion_get(&t);
        int start = distance_to_start_led(t.distance_cm);

        switch (s_led.mode) {
            case LED_MODE_STANDARD:         mode_standard(start);         break;
            case LED_MODE_RAINBOW:          mode_rainbow();               break;
            case LED_MODE_COLOR_WAVE:       mode_color_wave();            break;
            case LED_MODE_BREATHING:        mode_breathing();             break;
            case LED_MODE_SOLID:            mode_solid();                 break;
            case LED_MODE_COMET:            mode_comet(start);            break;
            case LED_MODE_PULSE:            mode_pulse(start);            break;
            case LED_MODE_FIRE:             mode_fire();                  break;
            case LED_MODE_THEATER_CHASE:    mode_theater_chase();         break;
            case LED_MODE_DUAL_SCAN:        mode_dual_scan(start);        break;
            case LED_MODE_MOTION_PARTICLES: mode_motion_particles(start); break;
            default:                         mode_standard(start);         break;
        }

        led_strip_refresh(s_led.strip);
        s_led.step++;
        vTaskDelay(pdMS_TO_TICKS(16));  /* ~60 Hz */
    }
}

/* ------ init / reload ---------------------------------------------- */

static void load_settings_from_nvs(void) {
    uint32_t v;
    uint8_t  b;
    v = DEFAULT_LEDS;        settings_get_u32("led", "count", &v);
    if (v < 1) v = DEFAULT_LEDS;
    if (v > MAX_LEDS) v = MAX_LEDS;
    s_led.count = (uint16_t)v;

    b = 80;                  settings_get_u8("led", "br", &b);
    s_led.brightness = b;

    b = 255;                 settings_get_u8("led", "r", &b); s_led.base.r = b;
    b = 255;                 settings_get_u8("led", "g", &b); s_led.base.g = b;
    b = 255;                 settings_get_u8("led", "b", &b); s_led.base.b = b;

    b = 0;                   settings_get_u8("led", "mode", &b); s_led.mode = b;
    v = 30;                  settings_get_u32("led", "span", &v); s_led.span = v;

    int32_t i32 = 0;         settings_get_i32("led", "ctr", &i32); s_led.center_shift = i32;
    b = 0;                   settings_get_u8("led", "trail", &b);   s_led.trail = b;
    b = 1;                   settings_get_u8("led", "dirlt", &b);   s_led.dir_light = b != 0;
    b = 0;                   settings_get_u8("led", "bg", &b);      s_led.bg_mode = b != 0;
    b = 50;                  settings_get_u8("led", "espd", &b);    s_led.effect_speed = b;
    b = 50;                  settings_get_u8("led", "eint", &b);    s_led.effect_intensity = b;

    v = 30;  settings_get_u32("dist", "min", &v); s_led.min_cm = v;
    v = 300; settings_get_u32("dist", "max", &v); s_led.max_cm = v;
}

void led_engine_reload(void) {
    uint16_t prev_count = s_led.count;
    load_settings_from_nvs();
    if (s_led.count != prev_count) {
        ESP_LOGI(TAG, "LED count changed %u → %u; reallocating buffers", prev_count, s_led.count);
        if (s_led.fire_heat) { free(s_led.fire_heat); s_led.fire_heat = NULL; }
        if (s_led.prev_frame){ free(s_led.prev_frame); s_led.prev_frame = NULL; }
        s_led.fire_heat  = calloc(s_led.count, 1);
        s_led.prev_frame = calloc(s_led.count, sizeof(rgb_t));
        /* Strip resize requires re-init via the led_strip API. */
        led_strip_del(s_led.strip);
        led_strip_config_t scfg = {
            .strip_gpio_num = s_led.data_gpio,
            .max_leds = s_led.count,
            .led_model = LED_MODEL_WS2812,
            .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
            .flags.invert_out = false,
        };
        led_strip_rmt_config_t rcfg = { .clk_src = RMT_CLK_SRC_DEFAULT, .resolution_hz = 10*1000*1000, .flags.with_dma = false };
        ESP_ERROR_CHECK(led_strip_new_rmt_device(&scfg, &rcfg, &s_led.strip));
    }
}

esp_err_t led_engine_init(uint8_t data_gpio) {
    s_led.data_gpio = data_gpio;
    load_settings_from_nvs();

    s_led.fire_heat  = calloc(s_led.count, 1);
    s_led.prev_frame = calloc(s_led.count, sizeof(rgb_t));

    led_strip_config_t scfg = {
        .strip_gpio_num = data_gpio,
        .max_leds = s_led.count,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rcfg = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };
    esp_err_t err = led_strip_new_rmt_device(&scfg, &rcfg, &s_led.strip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "led_strip_new_rmt_device(GPIO%u): 0x%x", data_gpio, err);
        return err;
    }
    led_strip_clear(s_led.strip);
    led_strip_refresh(s_led.strip);

    ESP_LOGI(TAG, "LED engine: %u LEDs on GPIO %u, mode=%d, br=%u, base=(%u,%u,%u)",
             s_led.count, data_gpio, s_led.mode, s_led.brightness,
             s_led.base.r, s_led.base.g, s_led.base.b);

    xTaskCreate(render_task, "led_render", 6144, NULL, 4, NULL);
    return ESP_OK;
}
