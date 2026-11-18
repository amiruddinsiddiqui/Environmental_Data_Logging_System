#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "uart/uart2.h"
#include "wifi/wifi_sta.h"
#include "mqtt/mqtt_pub.h"

#include "sensor_data.h"


void app_main(void) {
    QueueHandle_t sensor_queue;

    /* Create queue */
    sensor_queue = xQueueCreate(10, sizeof(SensorMessage_t));

    if(sensor_queue == NULL)
        return;

    /* Start UART task */
    xTaskCreate(uart_task, "uart_task", 4096, sensor_queue, 5, NULL);

    /* Connect Wi-Fi */
    ESP_ERROR_CHECK(wifi_init());
    wifi_wait_connected();

    /* Start MQTT */
    mqtt_app_start();

    /* Start MQTT task */
    xTaskCreate(mqtt_task, "mqtt_task", 4096, sensor_queue, 5, NULL);

}