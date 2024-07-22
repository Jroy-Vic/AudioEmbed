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
#define TIM2_NVIC (((uint32_t)TIM2_IRQn) >> 5UL)	// Global Interrupt Enable
#define TIM2_PRIORITY 0x1				 // TIM2 Interrupt has Second Priority
#define TIM2_PRESCALER 0x2F				 // Scale Clock Down from 24MHz to 500kHz
#define TIM2_ARR 0xB					 // ARR Value to Create 48kHz Interrupt

/* Global Variable */
extern uint8_t Output_Flag;

/* Initialize TIM2 Timer */
void TIM_init(void);

/* Force Timer Reset */
void TIM_reset(void);


/* Disable TIM2 */
void TIM_disable(void);

/* Re-Enable TIM2 */
void TIM_enable(void);

#endif /* INC_TIM_H_ */
