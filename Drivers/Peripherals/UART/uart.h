/*
 * uart.h
 *
 *  Created on: Nov 14, 2026
 *      Author: asus
 */

#ifndef PERIPHERALS_UART_UART_H_
#define PERIPHERALS_UART_UART_H_

#include "stm32f4xx.h"
#include "stdint.h"


typedef struct UART_Config_t {
	USART_TypeDef *instance;
	uint32_t baudrate;
} UART_Config_t;

extern UART_Config_t usart2_config;
extern UART_Config_t usart1_config;

void gpioa_uart_init(uint8_t pin_rx, uint8_t pin_tx);
void uart_init(UART_Config_t * config, uint8_t pin_rx, uint8_t pin_tx);
void uart_tx(UART_Config_t* config, uint8_t data);

#endif /* PERIPHERALS_UART_UART_H_ */
