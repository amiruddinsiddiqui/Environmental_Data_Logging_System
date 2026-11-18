idf_component_register(
    SRCS
        "main.c"
        "uart/uart2.c"
        "wifi/wifi_sta.c"
        "mqtt/mqtt_pub.c"

    INCLUDE_DIRS
        "."
        "uart"
        "wifi"
        "mqtt"

    REQUIRES
        esp_driver_uart
        esp_wifi
        esp_event
        esp_netif
        nvs_flash
        esp_timer
        mqtt
)