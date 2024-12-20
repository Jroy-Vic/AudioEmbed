/*
 * TIM.h
 *
 *  Created on: Jul 18, 2024
 *      Author: vicer
 */

#ifndef INC_TIM_H_
#define INC_TIM_H_

#include "main.h"

/* MACROS */
#define TIM6_PRESCALER 17	// Scale Clock Down from 96MHz to 5.65MHz
#define TIM6_ARR 1			// ARR Value to Create 5.65MHz Interrupt

/* Initialize TIM2 Timer */
void TIM_init(void);

#endif /* INC_TIM_H_ */
