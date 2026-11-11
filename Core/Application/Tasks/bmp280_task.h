/*
 * bmp280_task.h
 *
 *  Created on: Nov 11, 2026
 *      Author: asus
 */

#ifndef APPLICATION_BMP280_TASK_H_
#define APPLICATION_BMP280_TASK_H_

////////////////////////// for uart data queue travel
typedef struct
{
    double temperature;
    double pressure;
} BMP280_Data_t;
////////////////////////////
void StartBMP280Task(void *argument);


#endif /* APPLICATION_BMP280_TASK_H_ */
