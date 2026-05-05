#include "settings.h"

#include <string.h>

#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "settings";
static const char *NS_BOARD = "board";

esp_err_t settings_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition needs reformat (err=0x%x); erasing", err);
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init failed: 0x%x", err);
    }
    return err;
}

/* Open helper that hides the open-mode boilerplate. */
static esp_err_t open_ns(const char *ns, nvs_open_mode_t mode, nvs_handle_t *out) {
    esp_err_t err = nvs_open(ns, mode, out);
    if (err == ESP_ERR_NVS_NOT_FOUND && mode == NVS_READONLY) {
        /* Namespace doesn't exist yet — that's fine for first-boot reads. */
        return ESP_ERR_NVS_NOT_FOUND;
    }
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "nvs_open(%s) failed: 0x%x", ns, err);
    }
    return err;
}

esp_err_t settings_get_board_id(char *out, size_t max) {
    if (!out || max == 0) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(NS_BOARD, NVS_READONLY, &h);
    if (err != ESP_OK) return err;
    size_t len = max;
    err = nvs_get_str(h, "id", out, &len);
    nvs_close(h);
    return err;
}

esp_err_t settings_set_board_id(const char *id) {
    if (!id) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(NS_BOARD, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_str(h, "id", id);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

esp_err_t settings_get_pin_override(const char *key, uint8_t *out) {
    if (!key || !out) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(NS_BOARD, NVS_READONLY, &h);
    if (err != ESP_OK) return err;
    err = nvs_get_u8(h, key, out);
    nvs_close(h);
    return err;
}

esp_err_t settings_set_pin_override(const char *key, uint8_t pin) {
    if (!key) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(NS_BOARD, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_u8(h, key, pin);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

esp_err_t settings_get_radar_kind(char *out, size_t max) {
    if (!out || max == 0) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(NS_BOARD, NVS_READONLY, &h);
    if (err != ESP_OK) return err;
    size_t len = max;
    err = nvs_get_str(h, "radar_kind", out, &len);
    nvs_close(h);
    return err;
}

esp_err_t settings_set_radar_kind(const char *kind) {
    if (!kind) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(NS_BOARD, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_str(h, "radar_kind", kind);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

/* ---- generic typed accessors -------------------------------------------- */

esp_err_t settings_get_str(const char *ns, const char *key, char *out, size_t max) {
    if (!ns || !key || !out || max == 0) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(ns, NVS_READONLY, &h);
    if (err != ESP_OK) return err;
    size_t len = max;
    err = nvs_get_str(h, key, out, &len);
    nvs_close(h);
    return err;
}

esp_err_t settings_set_str(const char *ns, const char *key, const char *val) {
    if (!ns || !key || !val) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(ns, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_str(h, key, val);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

esp_err_t settings_get_u32(const char *ns, const char *key, uint32_t *out) {
    if (!ns || !key || !out) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(ns, NVS_READONLY, &h);
    if (err != ESP_OK) return err;
    err = nvs_get_u32(h, key, out);
    nvs_close(h);
    return err;
}

esp_err_t settings_set_u32(const char *ns, const char *key, uint32_t v) {
    if (!ns || !key) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(ns, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_u32(h, key, v);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

esp_err_t settings_get_i32(const char *ns, const char *key, int32_t *out) {
    if (!ns || !key || !out) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(ns, NVS_READONLY, &h);
    if (err != ESP_OK) return err;
    err = nvs_get_i32(h, key, out);
    nvs_close(h);
    return err;
}

esp_err_t settings_set_i32(const char *ns, const char *key, int32_t v) {
    if (!ns || !key) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(ns, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_i32(h, key, v);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

esp_err_t settings_get_u8(const char *ns, const char *key, uint8_t *out) {
    if (!ns || !key || !out) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(ns, NVS_READONLY, &h);
    if (err != ESP_OK) return err;
    err = nvs_get_u8(h, key, out);
    nvs_close(h);
    return err;
}

esp_err_t settings_set_u8(const char *ns, const char *key, uint8_t v) {
    if (!ns || !key) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(ns, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_u8(h, key, v);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

esp_err_t settings_get_blob(const char *ns, const char *key, void *out, size_t *len) {
    if (!ns || !key || !out || !len) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(ns, NVS_READONLY, &h);
    if (err != ESP_OK) return err;
    err = nvs_get_blob(h, key, out, len);
    nvs_close(h);
    return err;
}

esp_err_t settings_set_blob(const char *ns, const char *key, const void *data, size_t len) {
    if (!ns || !key || !data) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = open_ns(ns, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_blob(h, key, data, len);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

/* ---- wifi namespace shortcuts ------------------------------------------- */
esp_err_t settings_get_wifi_ssid(char *o, size_t m) { return settings_get_str("wifi", "ssid", o, m); }
esp_err_t settings_set_wifi_ssid(const char *s)      { return settings_set_str("wifi", "ssid", s); }
esp_err_t settings_get_wifi_pass(char *o, size_t m) { return settings_get_str("wifi", "pass", o, m); }
esp_err_t settings_set_wifi_pass(const char *p)      { return settings_set_str("wifi", "pass", p); }
esp_err_t settings_get_hostname (char *o, size_t m) { return settings_get_str("wifi", "host", o, m); }
esp_err_t settings_set_hostname (const char *n)      { return settings_set_str("wifi", "host", n); }

/* ---- sys namespace shortcuts -------------------------------------------- */
esp_err_t settings_get_device_name(char *o, size_t m) { return settings_get_str("sys", "name", o, m); }
esp_err_t settings_set_device_name(const char *n)      { return settings_set_str("sys", "name", n); }

/* ---- auth namespace ----------------------------------------------------- */
esp_err_t settings_get_auth_hash(uint8_t hash[32], uint8_t salt[16]) {
    nvs_handle_t h;
    esp_err_t err = open_ns("auth", NVS_READONLY, &h);
    if (err != ESP_OK) return err;
    size_t hlen = 32, slen = 16;
    err = nvs_get_blob(h, "pw_hash", hash, &hlen);
    if (err == ESP_OK) err = nvs_get_blob(h, "pw_salt", salt, &slen);
    nvs_close(h);
    if (err == ESP_OK && (hlen != 32 || slen != 16)) return ESP_ERR_INVALID_SIZE;
    return err;
}

esp_err_t settings_set_auth_hash(const uint8_t hash[32], const uint8_t salt[16]) {
    nvs_handle_t h;
    esp_err_t err = open_ns("auth", NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_blob(h, "pw_hash", hash, 32);
    if (err == ESP_OK) err = nvs_set_blob(h, "pw_salt", salt, 16);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

esp_err_t settings_clear_auth(void) {
    nvs_handle_t h;
    esp_err_t err = open_ns("auth", NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    nvs_erase_key(h, "pw_hash");
    nvs_erase_key(h, "pw_salt");
    err = nvs_commit(h);
    nvs_close(h);
    return err;
}
