#ifndef WIFI_STA_H
#define WIFI_STA_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#define WIFI_CONNECTED_BIT BIT0

esp_err_t wifi_init(void);

void wifi_wait_connected(void);

#endif