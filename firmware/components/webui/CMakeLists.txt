idf_component_register(
    SRCS "webui.c"
    INCLUDE_DIRS "include"
    REQUIRES settings auth netmgr ota board
             esp_http_server esp_wifi esp_app_format esp_timer
             json log freertos esp_system
)
