/*
 * HPF.c
 *
 *  Created on: Dec 20, 2024
 *      Author: vicer
 */

#include "main.h"
#include <math.h>
#include "HPF.h"


/* Initialize First-Order High Pass Filter */
void HPF_init(HPF_t *filter, float corner_freq, float samp_freq) {
	/* Store Sample Frequency */
	filter->samp_freq = samp_freq;

	/* Store Corner Frequency and Calculate Coefficients for Filter */
	HPF_setCorner(filter, corner_freq);

	/* Reset Input/Output */
	filter->input = 0.0f;
	filter->output = 0.0f;
}


/* Adjust the Corner Frequency of HPF,
 * then Calculate Required Coefficients
 *
 * */
void HPF_setCorner(HPF_t *filter, float corner_freq) {
	/* Restrict Corner Frequency to be within 0 - (samp_freq / 2) [Nyquist Law] */
	if (corner_freq > (filter->samp_freq * 0.5f)) {
		corner_freq = (filter->samp_freq * 0.5f);
	} else if (corner_freq < 0.0f) {
		corner_freq = 0.0f;
	}

	/* Calculate the Required Coefficients */
	float coeff = ((2 * M_PI * corner_freq) / filter->samp_freq);
	filter->coeff = (1.0f / (1.0f + coeff));
}


/* Apply Filter to Input */
/* Discrete IIR Filter:
 * Vout[n] = (C * (Vin[n] - Vin[n-1] + Vout[n-1]))
 * */
float HPF_apply(HPF_t *filter, float input) {
	/* Output Filtered Input */
	filter->output = (filter->coeff * (input - filter->input + filter->output));

	/* Save Current Input */
	filter->input = input;

	/* Restrict Output to -1 and +1 */
	if (filter->output > 1.0f) {
		filter->output = 1.0f;
	} else if (filter->output < -1.0f) {
		filter->output = -1.0f;
	}

	/* Output Filtered Value */
	return filter->output;
}
