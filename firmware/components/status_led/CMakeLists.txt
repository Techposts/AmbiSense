idf_component_register(
    SRCS "status_led.c"
    INCLUDE_DIRS "include"
    REQUIRES driver log freertos
)
