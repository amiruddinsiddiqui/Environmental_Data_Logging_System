/*
 * bmp280.h
 *
 *  Created on: Nov 9, 2026
 *      Author: asus
 */

#ifndef SENSORS_BMP280_BMP280_H_
#define SENSORS_BMP280_BMP280_H_

#include "stdint.h"



void bmp280_init(void);
void bmp280_merge_bytes_to_adc(void);
void bmp280_read_calibration(void);
void bmp280_extract_calibration(void);
double bmp280_compensate_T_double(void);
double bmp280_compensate_P_double(void);
void bmp280_read_measurements(void);

#endif /* SENSORS_BMP280_BMP280_H_ */
