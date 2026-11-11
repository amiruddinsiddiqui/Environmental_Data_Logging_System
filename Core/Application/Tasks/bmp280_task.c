/*
 * bmp280_task.c
 *
 *  Created on: Nov 11, 2026
 *      Author: asus
 */


#include "bmp280_task.h"
#include "bmp280.h"
#include "cmsis_os.h"
#include "app.h"



void StartBMP280Task(void *argument) {
    BMP280_Data_t data;

    for (;;) {
        bmp280_read_measurements(); // Triggers the DMA read

        // Sleep until the ISR wakes us up (Flag 0x01)
        uint32_t flags = osThreadFlagsWait(0x01U, osFlagsWaitAny, 100);

        if (flags == 0x01U) { // Ensure it woke up from the flag
            bmp280_merge_bytes_to_adc();

            data.temperature = bmp280_compensate_T_double();
            data.pressure = bmp280_compensate_P_double();

            osMessageQueuePut(bmp280DataQueueHandle, &data, 0, 0);
        }

        osDelay(5000);
    }
}



