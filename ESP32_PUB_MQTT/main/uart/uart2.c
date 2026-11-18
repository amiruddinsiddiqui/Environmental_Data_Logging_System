#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/uart.h"
#include "esp_log.h"

#include "uart2.h"
#include "sensor_data.h"

#define UART2 UART_NUM_2

static const char *TAG = "UART2";

void uart_task(void *arg)
{
    QueueHandle_t sensor_queue = (QueueHandle_t)arg;

    uart_driver_install(UART2, 2048, 2048, 0, NULL, 0);

    uart_config_t config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_param_config(UART2, &config);
    uart_set_pin(UART2, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    uint8_t rx[80];
    SensorMessage_t msg;
    int index = 0;

    while (1)
    {
        int len = uart_read_bytes(UART2, rx, sizeof(rx), pdMS_TO_TICKS(100));

        for (int i = 0; i < len; i++)
        {
            if (rx[i] == '\n')
            {
                msg.data[index] = '\0';

                if (index > 0)
                {
                    if (xQueueSend(sensor_queue, &msg, 0) == pdPASS)
                        ESP_LOGI(TAG, "Received: %s", msg.data);
                }

                index = 0;
            }
            else if (rx[i] != '\r' && index < SENSOR_MSG_SIZE - 1)
            {
                msg.data[index++] = (char)rx[i];
            }
        }
    }
}


