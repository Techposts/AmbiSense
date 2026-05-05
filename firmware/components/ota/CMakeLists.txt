idf_component_register(
    SRCS "ota.c"
    INCLUDE_DIRS "include"
    REQUIRES app_update esp_app_format log freertos
)
