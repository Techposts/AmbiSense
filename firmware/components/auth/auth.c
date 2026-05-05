#include "auth.h"

#include <string.h>
#include <time.h>
#include <stdio.h>

#include "esp_log.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "mbedtls/pkcs5.h"
#include "mbedtls/md.h"

#include "settings.h"

static const char *TAG = "auth";

#define PBKDF2_ROUNDS 250000
#define SALT_LEN 16
#define HASH_LEN 32
#define MAX_SESSIONS 8
#define SESSION_TTL_MS (24ULL * 60ULL * 60ULL * 1000ULL)  /* 24h */

typedef struct {
    bool active;
    uint8_t token[AUTH_TOKEN_LEN];
    uint64_t expires_ms;
} session_t;

static struct {
    bool inited;
    bool enabled;
    uint8_t hash[HASH_LEN];
    uint8_t salt[SALT_LEN];
    SemaphoreHandle_t lock;
    session_t sessions[MAX_SESSIONS];
} s_auth;

static uint64_t now_ms(void) {
    return (uint64_t)xTaskGetTickCount() * portTICK_PERIOD_MS;
}

static void hex_encode(const uint8_t *in, size_t n, char *out) {
    static const char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < n; ++i) {
        out[i*2]   = hex[in[i] >> 4];
        out[i*2+1] = hex[in[i] & 0x0f];
    }
    out[n*2] = '\0';
}

static int hex_decode(const char *in, uint8_t *out, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        char hi = in[i*2], lo = in[i*2+1];
        if (!hi || !lo) return -1;
        int v = 0;
        for (int k = 0; k < 2; ++k) {
            char c = (k == 0) ? hi : lo;
            int d;
            if (c >= '0' && c <= '9') d = c - '0';
            else if (c >= 'a' && c <= 'f') d = c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') d = c - 'A' + 10;
            else return -1;
            v = (v << 4) | d;
        }
        out[i] = (uint8_t)v;
    }
    return 0;
}

static int pbkdf2(const char *plaintext, const uint8_t salt[SALT_LEN], uint8_t out[HASH_LEN]) {
    const mbedtls_md_info_t *info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!info) return -1;
#if defined(MBEDTLS_VERSION_MAJOR) && MBEDTLS_VERSION_MAJOR >= 3
    return mbedtls_pkcs5_pbkdf2_hmac_ext(MBEDTLS_MD_SHA256,
        (const unsigned char *)plaintext, strlen(plaintext),
        salt, SALT_LEN, PBKDF2_ROUNDS, HASH_LEN, out);
#else
    mbedtls_md_context_t ctx;
    mbedtls_md_init(&ctx);
    int rc = mbedtls_md_setup(&ctx, info, 1);
    if (rc == 0) {
        rc = mbedtls_pkcs5_pbkdf2_hmac(&ctx,
            (const unsigned char *)plaintext, strlen(plaintext),
            salt, SALT_LEN, PBKDF2_ROUNDS, HASH_LEN, out);
    }
    mbedtls_md_free(&ctx);
    return rc;
#endif
}

esp_err_t auth_init(void) {
    if (s_auth.inited) return ESP_OK;
    s_auth.lock = xSemaphoreCreateMutex();
    s_auth.inited = true;

    if (settings_get_auth_hash(s_auth.hash, s_auth.salt) == ESP_OK) {
        s_auth.enabled = true;
        ESP_LOGI(TAG, "Auth enabled (password configured)");
    } else {
        s_auth.enabled = false;
        ESP_LOGI(TAG, "Auth DISABLED (no password set — open access on local network)");
    }
    return ESP_OK;
}

bool auth_is_enabled(void) {
    return s_auth.enabled;
}

esp_err_t auth_set_password(const char *plaintext) {
    if (!plaintext || !plaintext[0]) {
        settings_clear_auth();
        s_auth.enabled = false;
        memset(s_auth.hash, 0, HASH_LEN);
        memset(s_auth.salt, 0, SALT_LEN);
        auth_revoke_all();
        ESP_LOGI(TAG, "Password cleared; auth disabled");
        return ESP_OK;
    }
    if (strlen(plaintext) < 8) return ESP_ERR_INVALID_ARG;

    uint8_t salt[SALT_LEN];
    esp_fill_random(salt, SALT_LEN);
    uint8_t hash[HASH_LEN];
    if (pbkdf2(plaintext, salt, hash) != 0) return ESP_FAIL;

    esp_err_t err = settings_set_auth_hash(hash, salt);
    if (err != ESP_OK) return err;
    memcpy(s_auth.hash, hash, HASH_LEN);
    memcpy(s_auth.salt, salt, SALT_LEN);
    s_auth.enabled = true;
    auth_revoke_all();  /* invalidate any pre-existing sessions */
    ESP_LOGI(TAG, "Password set; auth enabled");
    return ESP_OK;
}

bool auth_check_password(const char *plaintext) {
    if (!s_auth.enabled || !plaintext) return false;
    uint8_t cand[HASH_LEN];
    if (pbkdf2(plaintext, s_auth.salt, cand) != 0) return false;
    /* Constant-time compare */
    uint8_t diff = 0;
    for (size_t i = 0; i < HASH_LEN; ++i) diff |= cand[i] ^ s_auth.hash[i];
    return diff == 0;
}

esp_err_t auth_issue_session(char token_hex_out[AUTH_TOKEN_HEX_LEN]) {
    xSemaphoreTake(s_auth.lock, portMAX_DELAY);
    int slot = -1;
    uint64_t now = now_ms();
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!s_auth.sessions[i].active || s_auth.sessions[i].expires_ms < now) {
            slot = i; break;
        }
    }
    if (slot < 0) {
        /* All slots occupied & valid — evict oldest. */
        slot = 0;
        for (int i = 1; i < MAX_SESSIONS; ++i) {
            if (s_auth.sessions[i].expires_ms < s_auth.sessions[slot].expires_ms) slot = i;
        }
    }
    esp_fill_random(s_auth.sessions[slot].token, AUTH_TOKEN_LEN);
    s_auth.sessions[slot].expires_ms = now + SESSION_TTL_MS;
    s_auth.sessions[slot].active = true;
    hex_encode(s_auth.sessions[slot].token, AUTH_TOKEN_LEN, token_hex_out);
    xSemaphoreGive(s_auth.lock);
    return ESP_OK;
}

bool auth_check_session(const char *token_hex) {
    if (!token_hex || strlen(token_hex) < AUTH_TOKEN_LEN * 2) return false;
    uint8_t tok[AUTH_TOKEN_LEN];
    if (hex_decode(token_hex, tok, AUTH_TOKEN_LEN) < 0) return false;

    xSemaphoreTake(s_auth.lock, portMAX_DELAY);
    uint64_t now = now_ms();
    bool ok = false;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!s_auth.sessions[i].active) continue;
        if (s_auth.sessions[i].expires_ms < now) { s_auth.sessions[i].active = false; continue; }
        uint8_t diff = 0;
        for (size_t k = 0; k < AUTH_TOKEN_LEN; ++k) diff |= tok[k] ^ s_auth.sessions[i].token[k];
        if (diff == 0) { ok = true; break; }
    }
    xSemaphoreGive(s_auth.lock);
    return ok;
}

void auth_revoke(const char *token_hex) {
    if (!token_hex) return;
    uint8_t tok[AUTH_TOKEN_LEN];
    if (hex_decode(token_hex, tok, AUTH_TOKEN_LEN) < 0) return;
    xSemaphoreTake(s_auth.lock, portMAX_DELAY);
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (s_auth.sessions[i].active &&
            memcmp(tok, s_auth.sessions[i].token, AUTH_TOKEN_LEN) == 0) {
            s_auth.sessions[i].active = false;
        }
    }
    xSemaphoreGive(s_auth.lock);
}

void auth_revoke_all(void) {
    if (!s_auth.lock) return;
    xSemaphoreTake(s_auth.lock, portMAX_DELAY);
    for (int i = 0; i < MAX_SESSIONS; ++i) s_auth.sessions[i].active = false;
    xSemaphoreGive(s_auth.lock);
}
