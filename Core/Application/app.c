/*
 * i2c.c
 *
 *  Created on: Nov 11, 2026
 *      Author: asus
 */

#include "stm32f4xx.h"
#include "cmsis_os.h"

#include "clockconfig.h"
#include "i2c.h"
#include "dma.h"
#include "bmp280.h"
#include "uart.h"
#include "app.h"

#include "Tasks/bmp280_task.h"

#include "bmp280_task.h"
#include "uart_task.h"


osThreadId_t bmp280TaskHandle;
osThreadId_t uartTaskHandle;
osMessageQueueId_t bmp280DataQueueHandle;

const osThreadAttr_t bmp280Task_attributes = {
    .name = "bmp280Task",
    .stack_size = 512 * 4,
    .priority = osPriorityNormal,
};

const osThreadAttr_t uartTask_attributes = {
    .name = "uartTask",
    .stack_size = 256 * 4,
    .priority = osPriorityNormal,
};

const osMessageQueueAttr_t bmp280DataQueue_attributes = {
    .name = "bmp280DataQueue"
};



void app(void) {

    // initialize hardware peripherals
    SysClockConfig();
    SystemCoreClock = 84000000UL; // inform CMSIS of the new frequency
    i2c_init();
    dma_init();
    uart_init();
    bmp280_init();

    // initialize RTOS
    osKernelInitialize();

    // create queue
    bmp280DataQueueHandle = osMessageQueueNew(
        5,
        sizeof(BMP280_Data_t),
        &bmp280DataQueue_attributes
    );

    // create task
    bmp280TaskHandle = osThreadNew(
        StartBMP280Task,
        NULL,
        &bmp280Task_attributes
    );


    uartTaskHandle = osThreadNew(
        StartUARTTask,
        NULL,
        &uartTask_attributes
    );

    // start scheduler
    osKernelStart();

    while (1);

}


