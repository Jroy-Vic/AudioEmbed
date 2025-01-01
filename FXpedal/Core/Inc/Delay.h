/*
 * Delay.h
 *
 *  Created on: Jan 1, 2025
 *      Author: vicer
 */

#ifndef INC_DELAY_H_
#define INC_DELAY_H_

#include <stdint.h>

/* MACROS */
#define DELAY_MAX_LEN 10000
#define DELAY_TIME_MS 500.0f
#define DELAY_MIX 0.5f
#define DELAY_FEEDBACK 0.5f

typedef struct {

	/* Settings */
	float mix;	// [0] = Dry, [1] = Wet
	float feedback;

	/* Delay Line Buffer and Index */
	float line[DELAY_MAX_LEN];
	uint32_t lineIDX;

	/* Delay Line Length (Delay Time = Delay Line Length / Sample Rate) */
	uint32_t lineLen;

	/* Output */
	float output;

} Delay_t;


/* Initialize Delay Filter */
void Delay_init(Delay_t *df, float delayTime_ms, float mix,
				float feedback, float sampleRate_Hz);

/* Calculate Delay Output */
float Delay_update(Delay_t *df, float input);

/* Set Line Length */
void Delay_setLength(Delay_t *df, float delayTime_ms, float sampleRate_Hz);

#endif /* INC_DELAY_H_ */
