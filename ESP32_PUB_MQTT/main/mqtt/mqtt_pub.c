#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_event.h"

#include "mqtt_pub.h"
#include "sensor_data.h"


#define MQTT_DATA_TOPIC "env_monitor/sensor/data"
#define MQTT_CONNECTED_BIT BIT0

#define MQTT_BROKER_URI "mqtt://broker.emqx.io"

static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t mqtt_client;
static EventGroupHandle_t mqtt_event_group;


static void mqtt_event_handler(void *arg, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
    if (event_id == MQTT_EVENT_CONNECTED) {
        ESP_LOGI(TAG, "MQTT Connected");
        xEventGroupSetBits(mqtt_event_group, MQTT_CONNECTED_BIT);
    } else if (event_id == MQTT_EVENT_DISCONNECTED) {
        ESP_LOGW(TAG, "MQTT Disconnected");
        xEventGroupClearBits(mqtt_event_group, MQTT_CONNECTED_BIT);
    } else if (event_id == MQTT_EVENT_ERROR) {
        esp_mqtt_event_handle_t event = event_data;
        ESP_LOGE(TAG, "MQTT Error type: %d", event->error_handle->error_type);
    }
}


void mqtt_app_start(void) {
    mqtt_event_group = xEventGroupCreate();

    esp_mqtt_client_config_t config = {
        .broker.address.uri = MQTT_BROKER_URI,
        .session.keepalive = 30,
    };

    mqtt_client = esp_mqtt_client_init(&config);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}


void mqtt_task(void *arg) {
    QueueHandle_t sensor_queue = (QueueHandle_t)arg;

    SensorMessage_t msg;
    float temperature, pressure;
    char json[128];

    while (1) {
        xEventGroupWaitBits(mqtt_event_group, MQTT_CONNECTED_BIT, pdFALSE, pdTRUE, portMAX_DELAY);

        if (xQueueReceive(sensor_queue, &msg, portMAX_DELAY) == pdPASS) {
            if (sscanf(msg.data, "%f,%f", &temperature, &pressure) == 2) {
                snprintf(json, sizeof(json),
                         "{\"temperature\":%.2f,\"pressure\":%.2f}",
                         (double)temperature, (double)pressure);

                int id = esp_mqtt_client_publish(mqtt_client, MQTT_DATA_TOPIC, json, 0, 1, 0);

                if (id >= 0)
                    ESP_LOGI(TAG, "Published: %s", json);
                else
                    ESP_LOGW(TAG, "Publish failed");
            }
            else {
                ESP_LOGE(TAG, "Invalid data: %s", msg.data);
            }

        }

    }

}


