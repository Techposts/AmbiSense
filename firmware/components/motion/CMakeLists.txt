idf_component_register(
    SRCS "motion.c" "motion_kalman.c"
    INCLUDE_DIRS "include"
    REQUIRES radar settings esp_timer log freertos
)
