/*
 * ADC.h
 *
 *  Created on: Jul 18, 2024
 *      Author: vicer
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

#include "main.h"
#define TWENTYU_DELAY 80
#define ADC_NVIC (((uint32_t) ADC1_IRQn) >> 5UL)
#define ADC_NVIC_PRIORITY 0x0

/* Global Variables */
extern uint16_t GtrSamp_DigVal;
extern uint8_t Input_Flag;

/* Initialize ADC Peripheral */
void ADC_init(void);

/* Begin a New Conversion */
void ADC_collect(void);

#endif /* INC_ADC_H_ */
