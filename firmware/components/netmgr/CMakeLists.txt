idf_component_register(
    SRCS "netmgr.c"
    INCLUDE_DIRS "include"
    REQUIRES settings esp_wifi esp_netif esp_event nvs_flash mdns lwip log
)
