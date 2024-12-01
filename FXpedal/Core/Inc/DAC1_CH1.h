/*
 * DAC1_CH1.h
 *
 *  Created on: Aug 31, 2024
 *      Author: vicer
 */

#ifndef INC_DAC1_CH1_H_
#define INC_DAC1_CH1_H_

#include "main.h"

/* Macros */
#define BITMASK 0xFFF
#define DAC_CAL 0x3
#define DAC_SHIFT 0xC
#define MAXVOLT 330
#define MAXBIT 4095


/* Initialize and Configure DAC1 Peripheral */
void DAC_init(void);

#endif /* INC_DAC1_CH1_H_ */
