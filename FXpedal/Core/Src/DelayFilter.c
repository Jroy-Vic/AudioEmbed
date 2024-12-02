/*
 * DelayFilter.c
 *
 *  Created on: Dec 2, 2024
 *      Author: vicer
 */

#include "main.h"
#include "DelayFilter.h"
#include <stdlib.h>

/* Delay Effect Filter to produce echo effect
 * memBuffSize is used to adjust duration of delay;
 * attenuation (%) is used to adjust effect strength */


/* Initialize Delay Filter
 * cutoff is in percentage (%) */
void Delay_Filter_init(DelayFilter_t *dft, uint16_t size, float cutoff) {

	/* Dynamically Create Empty Memory Buffer */
	float *memBuff = (float*) calloc(size, sizeof(float));

	/* Instantiate Delay Filter Object */
	dft->memBuffPtr = memBuff;
	dft->memBuffSize = size;
	dft->attenuation = 0.1f;
//	dft->attenuation = (((PEAK_ATTENUATION - cutoff) / (size - 1)) / 100);
}

/* Accumulate Data in Memory */
void Delay_Filter_store(DelayFilter_t *dft, float data) {

	/* Right Shift Memory Buffer */
	for (uint16_t i = (dft->memBuffSize - 1); i > 0; i--) {
		(dft->memBuffPtr)[i] = (dft->memBuffPtr)[i - 1];
	}

	/* Store Data in Queue Memory Buffer */
	*(dft->memBuffPtr) = data;
}

/* Output Delay Effect */
float Delay_Filter_output(DelayFilter_t *dft) {

	/* Accumulate Attenuated Signal from Memory Buffer */
	float *tempPtr = dft->memBuffPtr;
	float output = 0x0;
	for (uint16_t i = 0x0; i < dft->memBuffSize; i++) {
		output += ((*tempPtr++) * (PEAK_ATTENUATION -
				  (i * dft->attenuation)));
	}

	/* Output Delayed Signal */
	return output;
}

/* Apply Delay Effect to Signal */
float Delay_Filter_apply(DelayFilter_t *dft, float data) {

	/* Store Data to Memory Buffer */
	Delay_Filter_store(dft, data);

	/* Output Data */
	return Delay_Filter_output(dft);
}