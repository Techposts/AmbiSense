#include "ota.h"

#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_app_desc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ota";

struct ota_session_s {
    esp_ota_handle_t handle;
    const esp_partition_t *target;
    size_t expected;
    size_t written;
};

ota_session_t *ota_begin(size_t expected_size) {
    const esp_partition_t *next = esp_ota_get_next_update_partition(NULL);
    if (!next) { ESP_LOGE(TAG, "no OTA partition"); return NULL; }

    ota_session_t *s = calloc(1, sizeof(*s));
    if (!s) return NULL;
    s->target = next;
    s->expected = expected_size;

    esp_err_t err = esp_ota_begin(next, OTA_WITH_SEQUENTIAL_WRITES, &s->handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin: 0x%x", err);
        free(s);
        return NULL;
    }
    ESP_LOGI(TAG, "OTA begin: partition '%s' offset 0x%lx, expected %u bytes",
             next->label, (unsigned long)next->address, (unsigned)expected_size);
    return s;
}

esp_err_t ota_write(ota_session_t *s, const void *data, size_t len) {
    if (!s) return ESP_ERR_INVALID_STATE;
    esp_err_t err = esp_ota_write(s->handle, data, len);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_write @ %u: 0x%x", (unsigned)s->written, err);
        ota_abort(s);
        return err;
    }
    s->written += len;
    return ESP_OK;
}

esp_err_t ota_finish(ota_session_t *s) {
    if (!s) return ESP_ERR_INVALID_STATE;
    esp_err_t err = esp_ota_end(s->handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_end: 0x%x (validation failed)", err);
        free(s);
        return err;
    }
    err = esp_ota_set_boot_partition(s->target);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "set_boot_partition: 0x%x", err);
        free(s);
        return err;
    }
    ESP_LOGI(TAG, "OTA committed: %u bytes to '%s'. Rebooting in 1 s.",
             (unsigned)s->written, s->target->label);
    free(s);
    /* Defer reboot so the HTTP response can flush. */
    extern void _ota_reboot_task(void *);
    xTaskCreate(_ota_reboot_task, "ota_reboot", 2048, NULL, 5, NULL);
    return ESP_OK;
}

void _ota_reboot_task(void *arg) {
    (void)arg;
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
}

void ota_abort(ota_session_t *s) {
    if (!s) return;
    esp_ota_abort(s->handle);
    free(s);
}

esp_err_t ota_mark_valid(void) {
    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t state;
    if (esp_ota_get_state_partition(running, &state) == ESP_OK) {
        if (state == ESP_OTA_IMG_PENDING_VERIFY) {
            ESP_LOGI(TAG, "Marking running image as valid (rollback armed → defused)");
            return esp_ota_mark_app_valid_cancel_rollback();
        }
    }
    return ESP_OK;
}
