/*
 * i2c.h
 *
 *  Created on: Nov 8, 2026
 *      Author: asus
 */

#ifndef PERIPHERALS_I2C_I2C_H_
#define PERIPHERALS_I2C_I2C_H_

#include "stdint.h"

void i2c_init(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_address(uint8_t addr);
void i2c_write(uint8_t data);
void i2c_read(uint8_t address, uint8_t *buffer, uint8_t size);


#endif /* PERIPHERALS_I2C_I2C_H_ */
