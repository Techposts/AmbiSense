idf_component_register(
    SRCS "topology.c"
    INCLUDE_DIRS "include"
    REQUIRES settings esp_system log freertos
)
