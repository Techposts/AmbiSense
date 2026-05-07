idf_component_register(
    SRCS "webui.c"
    INCLUDE_DIRS "include"
    REQUIRES settings auth netmgr ota board led_engine radar motion presence status_led
             esp_http_server esp_wifi esp_app_format esp_timer
             json log freertos esp_system
    EMBED_FILES "ui.html.gz" "ui.html"
)
