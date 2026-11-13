/*
 * sd_storage.h
 *
 *  Created on: Nov 12, 2026
 *      Author: asus
 */

#ifndef APPLICATION_STORAGE_SD_STORAGE_H_
#define APPLICATION_STORAGE_SD_STORAGE_H_


#include "main.h"
#include "fatfs.h"
#include <stdbool.h>


void SD_Card_Init(void);
void SD_Log_Data(double temperature, double pressure);


#endif /* SD_STORAGE_H_ */
