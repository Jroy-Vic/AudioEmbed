/*
 * TIM.c
 *
 *  Created on: Jul 18, 2024
 *      Author: vicer
 */


#include "TIM.h"
#include "main.h"

/* Initialize TIM2 and TIM6 */
void TIM_init() {
	/* Enable TIM6 Clock (48MHz) */
	RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM6EN);

	/* Configure Timer as Hardware Trigger */
	TIM6->PSC = TIM6_PRESCALER;
	TIM6->ARR = TIM6_ARR;

	/* Configure TIM6 to Generate TRGO on Update Event (010) */
	TIM6->CR2 &= ~TIM_CR2_MMS;
	TIM6->CR2 |= TIM_CR2_MMS_1;

	/* Enable Trigger */
	TIM6->CR1 |= TIM_CR1_CEN;
}
