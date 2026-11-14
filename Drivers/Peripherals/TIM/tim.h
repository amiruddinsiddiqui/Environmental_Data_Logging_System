/*
 * tim.h
 *
 *  Created on: Nov 14, 2026
 *      Author: asus
 */

#ifndef PERIPHERALS_TIM_TIM_H_
#define PERIPHERALS_TIM_TIM_H_

#include "stdint.h"

extern volatile uint8_t timer_5s_flag;
void tim2_init(void);

#endif /* PERIPHERALS_TIM_TIM_H_ */
