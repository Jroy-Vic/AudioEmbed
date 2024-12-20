/*
 * Keypad.h
 *
 *  Created on: Dec 2, 2024
 *      Author: vicer
 */

#ifndef INC_KEYPAD_H_
#define INC_KEYPAD_H_

#include "main.h"

/* Initialize Keypad Peripheral */
void Keypad_init(void);

/* Reset All Columns to Polling State */
void Keypad_reset(void);

/* Output Keypad Value */
int8_t Keypad_read(void);


#endif /* INC_KEYPAD_H_ */
