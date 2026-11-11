#ifndef APP_H_
#define APP_H_


#include "cmsis_os.h"

extern osThreadId_t bmp280TaskHandle;
extern osThreadId_t uartTaskHandle;
extern osMessageQueueId_t bmp280DataQueueHandle;

void app(void);

#endif

