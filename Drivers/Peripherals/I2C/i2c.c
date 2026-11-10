/*
 * i2c.c
 *
 *  Created on: Nov 8, 2026
 *      Author: asus
 */


#include "stm32f4xx.h"
#include "i2c.h"
#include "dma.h"



// I2C1_SCL -> PB8
// I2C1_SDA -> PB9

static void gpio_i2c_init(void) {
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

	GPIOB->MODER &= ~((3U<<(2*8)) | (3U<<(2*9)));
	GPIOB->MODER |= ((2U << (2*8)) | (2U << (2*9))); // Alternate Function Mode

	/*
	 * Open-drain output means the GPIO pin can only actively pull the I2C line LOW (0) & it cannot drive it HIGH (1)
	 *
	 * - To send LOW: MCU pulls SDA/SCL to GND
	 * - To send HIGH: MCU releases the line, and an external pull-up resistor pulls it to 3.3V
	 */
	GPIOB->OTYPER |= ((1U << 8) | (1U<<9)); // Open drain output

	GPIOB->OSPEEDR &= ~((3U<<(2*8)) | (3U<<(2*9)));
	GPIOB->OSPEEDR |= ((1U << (2*8)) | (1U << (2*9))); // 01: Medium speed output switching

	GPIOB->PUPDR &= ~((3U << (2*8)) | (3U << (2*9)));
	GPIOB->PUPDR |=  ((1U << (2*8)) | (1U << (2*9))); // 01: Pull-up

	GPIOB->AFR[1] &= ~((15U << 0) | (15U << 4));
	GPIOB->AFR[1] |= ((4U << 0) | (4U << 4));

}

void i2c_init(void) {
	gpio_i2c_init();

	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

	I2C1->CR1 |= I2C_CR1_SWRST;  // 1: I2C Peripheral under reset state
	I2C1->CR1 &= ~I2C_CR1_SWRST; // 0: I2C Peripheral not under reset

	I2C1->CR2 =  (42U << 0U); // I2C INPUT CLK FREQ -> 42MHz

	I2C1->CCR &= ~(1U << 15);  // 0: standard mode I2C

	/*
	CCR = ( T_r(scl) + T_w(sclh) ) / T_pclk1
	CCR = (1000 + (4*1000))/ (1/42MHz)
	CCR = 210
	*/
	// CCR determines SCL frequency by dividing the APB1 clock
	// f_SCL = PCLK1 / (2 * CCR)         // f_scl = frequency_scl
	// f_SCL = 42 MHz / (2 * 210) = 100 kHz
	I2C1->CCR = (210U << 0);



	//	TRISE configures the maximum allowed SCL rise time
	//    Standard Mode maximum rise time = 1000 ns
	//
	//	TRISE = (T_r(scl) / T_pclk1) + 1
	//	TRISE = (1000ns/23.8095ns)+1
	//	TRISE = 43
	/*
	 * At 100 kHz, one complete SCL clock cycle = 10 us
	 * Maximum allowed rise time in Standard Mode = 1000 ns (1 us)
	 * The 1 us rise time is part of the 10 us clock period
	 */
	I2C1->TRISE = 43<<0U; // 43 is equivalent to 1000ns which is the max rise time for i2c standard mode comm

	I2C1->CR1 |= I2C_CR1_PE; // 1: Peripheral enable

}


void i2c_start(void) {
	I2C1->CR1 |= I2C_CR1_ACK;

	I2C1->CR1 |= I2C_CR1_START;
//	while(I2C1->CR1 & I2C_CR1_START);
	while(!(I2C1->SR1 & I2C_SR1_SB));
}


void i2c_stop(void) {
	I2C1->CR1 |= I2C_CR1_STOP;

	while (I2C1->CR1 & I2C_CR1_STOP);
}

void i2c_address(uint8_t addr) {
	I2C1->DR = addr;
	while(!(I2C1->SR1 & I2C_SR1_ADDR)); // wait until received address by slave is matched & acknowledges its address

	uint32_t temp;
	temp = I2C1->SR1;  // read SR1 first (required to clear ADDR)
	temp = I2C1->SR2;  // read SR2 next; ADDR flag is now cleared
}


void i2c_write(uint8_t data) {
	while(!(I2C1->SR1 & I2C_SR1_TXE)); // TxE: Data register empty (transmitters)
	I2C1->DR = data;
	while(!(I2C1->SR1 & I2C_SR1_BTF)); // wait until BTF bit is set, BTF: Byte transfer finished
}

void i2c_read(uint8_t address, uint8_t *buffer, uint8_t size) {
	uint8_t remaining = size;

	if(size == 1) {
		I2C1->DR = address;
		while(!(I2C1->SR1 & I2C_SR1_ADDR)); // wait until received address by slave is matched & acknowledges its address

		I2C1->CR1 &= ~I2C_CR1_ACK; // disable ACK so the master sends NACK after receiving the byte

		uint32_t temp;
		temp = I2C1->SR1;  // read SR1 first (required to clear ADDR)
		temp = I2C1->SR2;  // read SR2 next; ADDR flag is now cleared

		i2c_stop(); // generate STOP to end the transaction after the final byte

		while(!(I2C1->SR1 & I2C_SR1_RXNE)); // wait until RxNE is set, RxNE : 1 -> Data register not empty aka data received

		buffer[0] = I2C1->DR;
	} else {
		i2c_address(address);

		while(remaining > 2) {
			while(!(I2C1->SR1 & I2C_SR1_RXNE)); // wait until RxNE is set, RxNE : 1 -> Data register not empty aka data received

			buffer[size - remaining] = I2C1->DR;
			remaining--;
		}

		// read the 2nd last byte
		while(!(I2C1->SR1 & I2C_SR1_RXNE)); // wait until RxNE is set, RxNE : 1 -> Data register not empty aka data received
		buffer[size - remaining] = I2C1->DR;
		remaining--;

		I2C1->CR1 &= ~I2C_CR1_ACK; // disable ACK so the master sends NACK after receiving the 2nd byte to end the comm after receiving last byte

		i2c_stop(); // generate STOP to end the transaction after the final byte
		// read the last byte
		while(!(I2C1->SR1 & I2C_SR1_RXNE)); // wait until RxNE is set, RxNE : 1 -> Data register not empty aka data received
		buffer[size - remaining] = I2C1->DR;
	}
}



void i2c_read_dma(uint8_t address, uint8_t *buffer, uint8_t size) {
    I2C1->CR1 |= I2C_CR1_ACK;

    dma_rx_done = 0;

    dma_config((uint32_t)&I2C1->DR,
               (uint32_t)buffer,
               size);

    // enable LAST and DMA requests
    I2C1->CR2 |= I2C_CR2_LAST;
    I2C1->CR2 |= I2C_CR2_DMAEN;


    i2c_address(address);

}

