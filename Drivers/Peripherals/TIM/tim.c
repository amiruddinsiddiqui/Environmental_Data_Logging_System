/*
 * tim.c
 *
 *  Created on: Nov 14, 2026
 *      Author: asus
 */

#include "stm32f4xx.h"
#include "tim.h"


void tim2_init(void) {
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

	TIM2->CR1 &= ~TIM_CR1_DIR;    // 0: Counter used as upcounter

	TIM2->PSC = 8400-1;           // 84MHz/(8399+1) -> 10KHz/sec

	TIM2->ARR = 10000-1;          // 1Hz/sec

	TIM2->DIER |= TIM_DIER_UIE;   // 1: Update interrupt enabled

    TIM2->EGR = TIM_EGR_UG;       // generate update event to load prescaler

    TIM2->SR &= ~TIM_SR_UIF;      // clear update interrupt flag

	TIM2->CR1 |= TIM_CR1_CEN;     // enable counter

	NVIC_EnableIRQ(TIM2_IRQn);
}

static uint8_t sec_counter = 0;
volatile uint8_t timer_5s_flag;

void TIM2_IRQHandler(void) {
	if(TIM2->SR & TIM_SR_UIF) {
		TIM2->SR &= ~TIM_SR_UIF;  // clear the update interrupt flag

		sec_counter++;

		if(sec_counter >= 5) {
			sec_counter = 0;
			timer_5s_flag = 1;   // flag set every 5 seconds
		}
	}
}



