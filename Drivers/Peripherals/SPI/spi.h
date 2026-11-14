/*
 * spi.h
 *
 *  Created on: Nov 11, 2026
 *      Author: asus
 */

#ifndef PERIPHERALS_SPI_SPI_H_
#define PERIPHERALS_SPI_SPI_H_

#include "stdint.h"

void spi2_init(void);
void spi_cs(uint8_t state);
uint8_t spi_tx_n_rx(uint8_t data);


#endif /* PERIPHERALS_SPI_SPI_H_ */
