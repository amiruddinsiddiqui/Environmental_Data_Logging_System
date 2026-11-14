/*
 * i2c.c
 *
 *  Created on: Nov 11, 2026
 *      Author: asus
 */

#include "stm32f4xx.h"

#include "app.h"
#include "i2c.h"
#include "dma.h"
#include "spi.h"
#include "bmp280.h"
#include "uart.h"

#include "stdint.h"
#include "stdio.h"

#include "sd_storage.h"
#include "tim.h"




UART_Config_t usart2_config = {
		.instance = USART2,
		.baudrate = 115200U
};

UART_Config_t usart1_config = {
		.instance = USART1,
		.baudrate = 115200U
};

volatile uint8_t bmp280_in_progress = 0;

void app(void) {
    // initialize hardware peripherals
    // PA3  -> USART2_ RX  // PA2  -> USART2_ TX
    uart_init(&usart2_config, 3, 2);

    i2c1_init();
    dma1_init();
    spi2_init();
    bmp280_init();

    tim2_init();

    // PA10 -> USART1_ RX  // PA9  -> USART1_ TX
    uart_init(&usart1_config, 10, 9);

	SD_Card_Init();

	printf("[BOOT] System initialized\r\n");
	uart_tx(&usart2_config, 'A');
	uart_tx(&usart2_config, '\r');
	uart_tx(&usart2_config, '\n');

	 while (1) {
		// schedule BMP280 every 5 seconds
		if (timer_5s_flag && !bmp280_in_progress) {
			timer_5s_flag = 0;
			dma_rx_done = 0;
			bmp280_in_progress = 1;

			bmp280_read_measurements();
		}

		// process completed DMA reception
		if (bmp280_in_progress && dma_rx_done) {
			dma_rx_done = 0;
			bmp280_in_progress = 0;

			bmp280_merge_bytes_to_adc();

			double temperature = bmp280_compensate_T_double();
			double pressure = bmp280_compensate_P_double();

			printf("Temperature: %d C | Pressure: %d Pa\r\n",
			       (int)temperature, (int)pressure);

			// log data continuously to SD Card
			SD_Log_Data(temperature, pressure);
		}


	} // end of WHILE loop

}

