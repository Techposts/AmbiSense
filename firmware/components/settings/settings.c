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
