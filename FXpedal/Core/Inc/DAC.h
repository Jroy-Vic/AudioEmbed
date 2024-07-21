/*
 * DAC.h
 *
 *  Created on: Jul 18, 2024
 *      Author: vicer
 */

#ifndef INC_DAC_H_
#define INC_DAC_H_

#include "main.h"

/* Macros */
#define BITMASK 0xFFF
#define DAC_CAL 0x3
#define DAC_SHIFT 0xC
#define MAXVOLT 330
#define MAXBIT 4095


/* Initialize and Configure DAC Peripheral */
void DAC_init(void);

/* Send Data to the DAC */
void DAC_Write(uint16_t LUT_volt);

#endif /* INC_DAC_H_ */
