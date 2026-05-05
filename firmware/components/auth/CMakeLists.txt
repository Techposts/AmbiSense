idf_component_register(
    SRCS "auth.c"
    INCLUDE_DIRS "include"
    REQUIRES settings mbedtls log esp_system freertos
)
