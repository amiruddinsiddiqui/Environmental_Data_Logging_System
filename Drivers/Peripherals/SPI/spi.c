/*
 * spi.c
 *
 *  Created on: Nov 11, 2026
 *      Author: asus
 */

#include "stm32f4xx.h"
#include "spi.h"


/*
  SPI2_SCK  -> PB13
  SPI2_MISO -> PB14
  SPI2_MOSI -> PB15
  CS        -> PB1
  DC        -> PB12
  RESET     -> PB7
*/

static void spi_gpio_init(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    // configure CS (PB1), DC (PB12), RESET (PB7) as output
    GPIOB->MODER &= ~((3U << (2U * 1U)) |
                      (3U << (2U * 7U)) |
                      (3U << (2U * 12U)));

    GPIOB->MODER |=  ((1U << (2U * 1U)) |
                      (1U << (2U * 7U)) |
                      (1U << (2U * 12U)));

    // set CS and RESET HIGH, DC LOW initially
    GPIOB->BSRR = (1U << 1U) | (1U << 7U) | (1U << (12U + 16U));


	GPIOB->MODER &= ~((3U<<(2*13)) |
					  (3U<<(2*14)) |
					  (3U<<(2*15)));
	// 10 : Alternate Function
	GPIOB->MODER |=  ((2U<<(2*13)) |
					  (2U<<(2*14)) |
					  (2U<<(2*15)));

	GPIOB->OSPEEDR &= ~((3U<<(2*13)) |
					    (3U<<(2*14)) |
					    (3U<<(2*15)));
	// 10 : high speed
	GPIOB->OSPEEDR |=  ((2U<<(2*13)) |
					    (2U<<(2*14)) |
					    (2U<<(2*15)));
	// 00: No pull-up, pull-down
	GPIOB->PUPDR &= ~((3U<<(2*13)) |
					  (3U<<(2*14)) |
					  (3U<<(2*15)));

	GPIOB->AFR[1] &= ~((15U<<(4*5)) |
					   (15U<<(4*6)) |
					   (15U<<(4*7)));
	// 0101 : AF05 -> SPI2
	GPIOB->AFR[1] |=  ((5U<<(4*5)) |
					   (5U<<(4*6)) |
					   (5U<<(4*7)));
}

void spi2_init(void) {
	spi_gpio_init();

	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;

	SPI2->CR1 &= ~SPI_CR1_BR;       // 000: fPCLK/2 -> 42/2 = 21MHz
	SPI2->CR1 |= SPI_CR1_BR_1;

	SPI2->CR1 &= ~SPI_CR1_CPHA;     // 0: The first clock transition is the first data capture edge
	SPI2->CR1 &= ~SPI_CR1_CPOL;     // 0: CK to 0 when idle
	SPI2->CR1 &= ~SPI_CR1_DFF;      // 0: 8-bit data frame format is selected for tx/rx
	SPI2->CR1 &= ~SPI_CR1_LSBFIRST; // 0: MSB transmitted first
	SPI2->CR1 &= ~SPI_CR1_RXONLY;   // 0: RXONLY -> FULL DUPLEX(Tx & Rx)
	SPI2->CR1 |= SPI_CR1_MSTR;      // 1: Master configuration -> MCU as Master/Controller

	SPI2->CR1 |= SPI_CR1_SSM;    // Software slave management
	SPI2->CR1 |= SPI_CR1_SSI;    // Internal NSS high

	SPI2->CR1 |= SPI_CR1_SPE;       // 1: Peripheral enabled
}

void spi_cs(uint8_t state) {
	if(state) {
		GPIOB->BSRR = (1U << 1U);        // cs high -> stop communication with slave
	} else {
		GPIOB->BSRR = (1U << (1U+16U));  // cs low  -> start communication with slave
	}
}


/*
   One side bit goes out into MOSI via the shift register while at the same
   time MISO gets one bit on the same clock pulse.
*/
uint8_t spi_tx_n_rx(uint8_t data) {
	while(!(SPI2->SR & SPI_SR_TXE)); // wait untile Tx buffer empty bit is set(1)
	SPI2->DR = data;

	while(!(SPI2->SR & SPI_SR_RXNE)); // wait untile Rx buffer not empty bit is set(1) ie data is received
	return (uint8_t)SPI2->DR;
}



//// MOSI -> Transmitter
//void spi_tx(uint8_t data) {
//	while(!(SPI2->SR & SPI_SR_TXE)); // wait untile Tx buffer empty bit is set(1)
//	SPI2->DR = data;
//	while(!(SPI2->SR & SPI_SR_TXE));
//}
//
//// MISO -> Receiver
//uint8_t spi_rx(uint8_t data) {
//	while(!(SPI2->SR & SPI_SR_RXNE)); // wait untile Rx buffer not empty bit is set(1)
//	data = SPI2->DR;
////	while(!(SPI2->SR & SPI_SR_RXNE));
//	return data;
//}



