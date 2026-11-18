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

#include "debug.h"

#include "tftrender.h"


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

	if(DEBUG) {
		// PA3  -> USART2_ RX  // PA2  -> USART2_ TX
		uart_init(&usart2_config, 3, 2);
		LOG("[BOOT] USART2 initialized\r\n");
	}

    i2c1_init();
    LOG("[BOOT] I2C1 initialized\r\n");
    dma1_init();
    LOG("[BOOT] DMA1 initialized\r\n");
    spi2_init();
    LOG("[BOOT] SPI2 initialized\r\n");
    bmp280_init();
    LOG("[BOOT] BMP280 initialized\r\n");

    // PA10 -> USART1_ RX  // PA9  -> USART1_ TX
    uart_init(&usart1_config, 10, 9);
    LOG("[BOOT] USART1 initialized\r\n");

	SD_Card_Init();
	LOG("[BOOT] SD CARD initialized\r\n");

	TFT_Display_Init();
	LOG("[BOOT] ILI9341 Display initialized\r\n");

	tim2_init();
	LOG("[BOOT] TIM2 initialized\r\n");

	LOG("[BOOT] System initialized\r\n");

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


			// log data continuously to SD Card every 5 seconds
			SD_Log_Data(temperature, pressure);
			TFT_Display_Update();

			uart_tx_multi_data(&usart1_config);

			LOG("T: %s%d.%d C | P: %d.%d Pa\r\n", sign, t_whole, t_dec, p_whole, p_dec);

		}


	} // end of WHILE loop

}



