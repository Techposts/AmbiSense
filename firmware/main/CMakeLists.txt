idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES board settings status_led netmgr auth webui ota
             app_update esp_system log
)
