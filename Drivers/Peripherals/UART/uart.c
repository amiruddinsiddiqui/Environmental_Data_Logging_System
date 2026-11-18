/*
 * uart.c
 *
 *  Created on: Nov 14, 2026
 *      Author: asus
 */

#include "stm32f4xx.h"
#include "uart.h"
#include "stdio.h"
#include "sd_storage.h"


#define PCLK1		  42000000U
#define PCLK2         84000000U



static uint16_t compute_baud_rate(uint32_t pCLK, uint32_t baudrate);

int __io_putchar(int ch) {
	uart_tx(&usart2_config, (uint8_t)ch);
	return ch;
}

// PA3  -> USART1_ RX
// PA2  -> USART1_ TX

// PA10 -> USART1_ RX
// PA9  -> USART1_ TX

void gpioa_uart_init(uint8_t pin_rx, uint8_t pin_tx) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	GPIOA->MODER &= ~((3U << (pin_rx*2)) |
					  (3U << (pin_tx*2)));
	GPIOA->MODER |= ((2U << (pin_rx*2)) | // AF
					 (2U << (pin_tx*2)));

	GPIOA->OSPEEDR &= ~((3U << (pin_rx*2)) |
					    (3U << (pin_tx*2)));
	GPIOA->OSPEEDR |= ((1U << (pin_rx*2)) | // 01 -> Medium speed
					   (1U << (pin_tx*2)));

	if(pin_rx<=7 && pin_tx<=7) {
		GPIOA->AFR[0] &= ~((15U << (pin_rx*4)) |
						   (15U << (pin_tx*4)));
		GPIOA->AFR[0] |= ((07U << (pin_rx*4)) | // AF07
						  (07U << (pin_tx*4)));
	} else { // Configure PA9 and PA10 for Alternate Function 7 (AF7) = USART1
	    GPIOA->AFR[1] &= ~((15U << ((pin_rx - 8U) * 4U)) |
	                       (15U << ((pin_tx - 8U) * 4U)));

	    GPIOA->AFR[1] |= ((7U << ((pin_rx - 8U) * 4U)) |
	                      (7U << ((pin_tx - 8U) * 4U)));
	}


}

void uart_init(UART_Config_t * config, uint8_t pin_rx, uint8_t pin_tx) {
	gpioa_uart_init(pin_rx, pin_tx);

	USART_TypeDef *uart = config->instance;

	if(uart == USART1) {
		 RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
		 uart->BRR = compute_baud_rate(PCLK2, config->baudrate);
	} else if(uart == USART2) {
		RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
		uart->BRR = compute_baud_rate(PCLK1, config->baudrate);
	}

	uart->CR1 &= ~USART_CR1_UE; // disable uart

	uart->CR1 |= USART_CR1_TE;
	uart->CR1 |= USART_CR1_RE;

	uart->CR1 |= USART_CR1_UE;
}


// brr = (42000000+(115200/2)) / 115200
// brr = 365
// so, baudrate = (42000000/365) = ~ 115068 baud rate = 115200
static uint16_t compute_baud_rate(uint32_t pCLK, uint32_t baudrate) {
	return (pCLK + (baudrate / 2)) / baudrate;
}


void uart_tx(UART_Config_t* config, uint8_t data) {
	USART_TypeDef* uart = config->instance;

	while(!(uart->SR & USART_SR_TXE)); // wait until data is transferred/wait until DR register is empty
	uart->DR = data;
//	while(!(uart->SR & USART_SR_TC)); // wait until data frame transmission is complete
}

char tx_buffer[40];
void uart_tx_multi_data(UART_Config_t *config) {

    snprintf(tx_buffer, sizeof(tx_buffer),
             "%s%d.%02d,%d.%02d\r\n",
             sign, t_whole, t_dec, p_whole, p_dec);

    for(uint8_t i = 0; tx_buffer[i] != '\0'; i++) {
        uart_tx(config, (uint8_t)tx_buffer[i]);
    }

}



