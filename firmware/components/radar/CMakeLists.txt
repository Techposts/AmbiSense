idf_component_register(
    SRCS "radar.c" "radar_ld2450.c" "radar_sim.c"
    INCLUDE_DIRS "include"
    REQUIRES settings driver esp_timer log freertos
)
