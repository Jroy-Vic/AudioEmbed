/*
 * LPF.h
 *
 *  Created on: Jul 21, 2024
 *      Author: vicer
 */

#ifndef INC_LPF_H_
#define INC_LPF_H_

#include "main.h"

#define LPF_CORNER_FREQ 100000.0f

/* Define First-Order Low Pass Filter Struct */
typedef struct {
	/* Filtered Output */
	float output;

	/* Filter Coefficients */
	float coeff[2];

	/* Sample Frequency (Hz) */
	float samp_freq;

} LPF_t;

/* Initialize First-Order Low Pass Filter */
void LPF_init(LPF_t *filter, float corner_freq, float samp_freq);

/* Adjust the Corner Frequency of LPF */
void LPF_setCorner(LPF_t *filter, float corner_freq);

/* Apply Filter to Input */
float LPF_apply(LPF_t *filter, float input);


#endif /* INC_LPF_H_ */
