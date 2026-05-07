idf_component_register(
    SRCS "presence.c"
    INCLUDE_DIRS "include"
    REQUIRES radar settings esp_timer log freertos
)
