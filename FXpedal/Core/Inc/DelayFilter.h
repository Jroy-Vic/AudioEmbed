/*
 * DelayFilter.h
 *
 *  Created on: Dec 2, 2024
 *      Author: vicer
 */

#ifndef INC_DELAYFILTER_H_
#define INC_DELAYFILTER_H_

#include "main.h"

/* MACROS */
#define PEAK_ATTENUATION (100.0f)


/* Create Delay Filter */
typedef struct {
	/* Memory Buffer Pointer */
	float *memBuffPtr;

	/* Memory Buffer Size */
	uint16_t memBuffSize;

	/* Memory Attenuation Coefficient */
	float attenuation;
} DelayFilter_t;


/* Initialize Delay Filter */
void Delay_Filter_init(DelayFilter_t *dft, uint16_t size, float cutoff);

/* Accumulate Data in Memory */
void Delay_Filter_store(DelayFilter_t *dft, float data);

/* Output Delay Effect */
float Delay_Filter_output(DelayFilter_t *dft);

/* Apply Delay Effect to Signal */
float Delay_Filter_apply(DelayFilter_t *dft, float data);


#endif /* INC_DELAYFILTER_H_ */
