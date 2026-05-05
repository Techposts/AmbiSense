idf_component_register(
    SRCS "mesh.c"
    INCLUDE_DIRS "include"
    REQUIRES settings motion topology esp_wifi esp_event esp_timer log freertos
)
