/*
 * HPF.h
 *
 *  Created on: Dec 20, 2024
 *      Author: vicer
 */

#ifndef INC_HPF_H_
#define INC_HPF_H_

#include "main.h"

#define HPF_CORNER_FREQ 100.0f

/* Define First-Order High Pass Filter Struct */
typedef struct {
	/* Filtered Input/Output */
	float input, output;


	/* Filter Coefficient */
	float coeff;

	/* Sample Frequency (Hz) */
	float samp_freq;

} HPF_t;

/* Initialize First-Order High Pass Filter */
void HPF_init(HPF_t *filter, float corner_freq, float samp_freq);

/* Adjust the Corner Frequency of HPF */
void HPF_setCorner(HPF_t *filter, float corner_freq);

/* Apply Filter to Input */
float HPF_apply(HPF_t *filter, float input);

#endif /* INC_HPF_H_ */
