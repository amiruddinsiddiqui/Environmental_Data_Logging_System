#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "nvs_flash.h"

#include "wifi_sta.h"


/* Wi-Fi credentials */
#define WIFI_SSID       "********"
#define WIFI_PASSWORD   "************"


static const char *TAG = "WIFI_STA";

static EventGroupHandle_t wifi_event_group;
static esp_timer_handle_t reconnect_timer;

static uint8_t retry_count = 0;


/* Reconnection timer callback */
static void reconnect_callback(void *arg) {
    ESP_LOGI(TAG, "Attempting Wi-Fi reconnection...");
    esp_wifi_connect();
}


/* Schedule Wi-Fi reconnection */
static void schedule_reconnect(void) {
    uint32_t delay_ms = 1000U;

    if(retry_count < 5)
        delay_ms <<= retry_count;
    else
        delay_ms = 30000U;

    if(delay_ms > 30000U)
        delay_ms = 30000U;

    retry_count++;

    ESP_LOGW(TAG, "Reconnecting in %lu ms", (unsigned long)delay_ms);
    esp_timer_stop(reconnect_timer);
    ESP_ERROR_CHECK(esp_timer_start_once(reconnect_timer, (uint64_t)delay_ms * 1000ULL));
}


/* Wi-Fi event handler */
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if(event_base == WIFI_EVENT) {
        switch(event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "Wi-Fi started. Connecting...");
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
                // Inside wifi_event_handler under WIFI_EVENT_STA_DISCONNECTED:
                wifi_event_sta_disconnected_t *disconn = (wifi_event_sta_disconnected_t *)event_data;
                ESP_LOGW(TAG, "Wi-Fi disconnected, reason: %d", disconn->reason);
                schedule_reconnect();
                break;

            default:
                break;
        }
    }

    else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

        ESP_LOGI(TAG, "Wi-Fi connected!");
        ESP_LOGI(TAG, "IP Address: " IPSTR, IP2STR(&event->ip_info.ip));

        retry_count = 0;
        esp_timer_stop(reconnect_timer);
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}


/* Wi-Fi initialization */
esp_err_t wifi_init(void) {
    esp_err_t ret;

    /* Create event group */
    wifi_event_group = xEventGroupCreate();

    if(wifi_event_group == NULL) {
        ESP_LOGE(TAG, "Event group creation failed");
        return ESP_ERR_NO_MEM;
    }


    /* Initialize NVS */
    ret = nvs_flash_init();

    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    if(ret != ESP_OK)
        return ret;

    /* Initialize network interface */
    ESP_ERROR_CHECK(esp_netif_init());

    /* Create default event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Create Wi-Fi station interface */
    esp_netif_create_default_wifi_sta();

    /* Initialize Wi-Fi driver */
    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));

    /* Create reconnection timer */
    esp_timer_create_args_t timer_args = {
        .callback = reconnect_callback,
        .arg = NULL,
        .name = "wifi_reconnect"
    };

    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &reconnect_timer));

    /* Register Wi-Fi event handler */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL));

    /* Register IP event handler */
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL));

    /* Configure Wi-Fi credentials */

    wifi_config_t wifi_cfg = {
        .sta = {
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false, // Prevents handshake stalls with mobile hotspots
            },
        },
    };
    snprintf((char *)wifi_cfg.sta.ssid, sizeof(wifi_cfg.sta.ssid), "%s", WIFI_SSID);
    snprintf((char *)wifi_cfg.sta.password, sizeof(wifi_cfg.sta.password), "%s", WIFI_PASSWORD);

    /* Set station mode */
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    /* Store configuration in RAM */
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    /* Apply Wi-Fi configuration */
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));

        /* Start Wi-Fi */
    ESP_ERROR_CHECK(esp_wifi_start());

    /* Disable power saving AFTER esp_wifi_start() to stop Reason 15 drops */
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    ESP_LOGI(TAG, "Wi-Fi initialization completed");

    return ESP_OK;
}


/* Wait until Wi-Fi obtains an IP address */
void wifi_wait_connected(void) {
    ESP_LOGI(TAG, "Waiting for Wi-Fi connection...");

    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
    ESP_LOGI(TAG, "Wi-Fi connection established");
}


