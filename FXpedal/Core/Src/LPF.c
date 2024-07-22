/*
 * LPF.c
 *
 *  Created on: Jul 21, 2024
 *      Author: vicer
 */


#include "main.h"
#include <math.h>
#include "LPF.h"


/* Initialize First-Order Low Pass Filter */
void LPF_init(LPF_t *filter, float corner_freq, float samp_freq) {
	/* Store Sample Frequency */
	filter->samp_freq = samp_freq;

	/* Store Corner Frequency and Calculate Coefficients for Filter */
	LPF_setCorner(filter, corner_freq);

	/* Reset Output */
	filter->output = 0.0f;
}


/* Adjust the Corner Frequency of LPF,
 * then Calculate Required Coefficients
 *
 * */
void LPF_setCorner(LPF_t *filter, float corner_freq) {
	/* Restrict Corner Frequency to be within 0 - (samp_freq / 2) */
	if (corner_freq > (filter->samp_freq * 0.5f)) {
		corner_freq = (filter->samp_freq * 0.5f);
	} else if (corner_freq < 0.0f) {
		corner_freq = 0.0f;
	}

	/* Calculate the Required Coefficients */
	float coeff = ((2 * M_PI * corner_freq) / filter->samp_freq);
	filter->coeff[0] = (coeff / (1.0f + coeff));
	filter->coeff[1] = (1.0f / (1.0f + coeff));
}


/* Apply Filter to Input */
/* Discrete IIR Filter:
 * Vout[n] = ((C / (1 + C)) * Vin[n]) + ((1 / (1 + C) * Vout[n-1]))
 *
 * */
float LPF_apply(LPF_t *filter, float input) {
	/* Output Filtered Input */
	filter->output = ((filter->coeff[0] * input) + (filter->coeff[1] * filter->output));

	/* Restrict Output to -1 and +1 */
	if (filter->output > 1.0f) {
		filter->output = 1.0f;
	} else if (filter->output < -1.0f) {
		filter->output = -1.0f;
	}

	/* Output Filtered Value */
	return filter->output;
}
