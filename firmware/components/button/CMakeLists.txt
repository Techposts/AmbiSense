idf_component_register(
    SRCS "button.c"
    INCLUDE_DIRS "include"
    REQUIRES driver log freertos esp_timer
)
