idf_component_register(
    SRCS "led_engine.c"
    INCLUDE_DIRS "include"
    REQUIRES motion settings esp_timer log freertos
)
