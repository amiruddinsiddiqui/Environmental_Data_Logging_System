/*
 * clockconfig.c
 *
 *  Created on: Oct 28, 2026
 *      Author: asus
 */

#include "clockconfig.h"




#define PLLM        4
#define PLLN        84U
#define PLLP        0 // 00 -> 2

void SysClockConfig(void) {
	// Turns on the external high-speed oscillator, which provides 8 MHz HSE clock.
	// ENABLE HSE and wait for the HSE to become Ready
	RCC->CR |= RCC_CR_HSEON;
	while(!(RCC->CR & RCC_CR_HSERDY));

	// ENABLE POWER CLOCK PERIPHERAL and VOLTAGE REGULATOR
	RCC->APB1ENR |= (1U<<28);
	PWR->CR |= PWR_CR_VOS_1; // 10 -> scale 2 -> 84MHz // Configure voltage scaling for the 84Mhz sys clock freq

	// Configure the FLASH PREFETCH and the LATENCY Related Settings
	FLASH->ACR |= FLASH_ACR_ICEN | FLASH_ACR_PRFTEN | FLASH_ACR_DCEN | FLASH_ACR_LATENCY_2WS;

	// Configure the PRESCALARS -> HCLK, PCLK1, PCLK2
	RCC->CFGR &= ~(RCC_CFGR_HPRE |
	               RCC_CFGR_PPRE1 |
	               RCC_CFGR_PPRE2);
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;   // HPRE: AHB prescaler  AHB/1 => HCLk = 84mhz
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;  // PPRE1: APB Low speed prescaler (APB1):  84/2 = 42Mhz
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;  // PPRE2: APB high-speed prescaler (APB2): 84/1 = 84Mhz

	RCC->CR &= ~RCC_CR_PLLON;
	while (RCC->CR & RCC_CR_PLLRDY);

	// Configure the MAIN PLL
	RCC->PLLCFGR &= ~((0x3FU) | (0x1FFU << 6));
	RCC->PLLCFGR |= (PLLM << 0);
	RCC->PLLCFGR |= (PLLN << 6);

	RCC->PLLCFGR &= ~(3U << 16);
	RCC->PLLCFGR |= (PLLP << 16);
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE; // 1: HSE oscillator clock selected as PLL input

	// Enable the PLL and wait for it to become ready
	RCC->CR |= RCC_CR_PLLON;
	while(!(RCC->CR & RCC_CR_PLLRDY));

	// Select the PLL Clock Source and wait for it to be set
	RCC->CFGR &= ~RCC_CFGR_SW;
	RCC->CFGR |= RCC_CFGR_SW_PLL;  // Tell the MCU to use PLL as SYSCLK
	while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);

}


