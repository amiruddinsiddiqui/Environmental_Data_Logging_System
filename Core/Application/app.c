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



volatile uint8_t bmp280_in_progress = 0;
volatile uint8_t seconds_count = 0;

void app(void) {

    // initialize hardware peripherals
    i2c_init();
    dma_init();
    spi_init();
    uart_init();
    bmp280_init();

    timer_init();

	uart_print("[BOOT] System initialized\r\n");

	SD_Card_Init();

	 while (1) {
		// schedule BMP280 every 5 seconds
		if (timer_1s_flag && !bmp280_in_progress) {
			timer_1s_flag = 0;
			seconds_count++;

			if (seconds_count >= 5) {
				seconds_count = 0;
				dma_rx_done = 0;
				bmp280_read_measurements();
				bmp280_in_progress = 1;
			}
		}

		// process completed DMA reception
		if (bmp280_in_progress && dma_rx_done) {
			dma_rx_done = 0;
			bmp280_in_progress = 0;

			bmp280_merge_bytes_to_adc();

			double temperature = bmp280_compensate_T_double();
			double pressure = bmp280_compensate_P_double();

			uart_print_double(temperature);
			uart_print(", ");
			uart_print_double(pressure);
			uart_print("\r\n");

			// log data continuously to SD Card
			SD_Log_Data(temperature, pressure);
		}


	} // end of WHILE loop

}




