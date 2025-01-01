/*
 * Delay.c
 *
 *  Created on: Jan 1, 2025
 *      Author: vicer
 */


#include "main.h"
#include "Delay.h"


/* Initialize Delay Filter */
void Delay_init(Delay_t *df, float delayTime_ms, float mix, float feedback, float sampleRate_Hz) {

	/* Set Delay Line Length */
	Delay_setLength(df, delayTime_ms, sampleRate_Hz);

	/* Store Delay Setting */
	df->mix = mix;
	df->feedback = feedback;

	/* Clear Delay Line Circular Buffer, Reset Index */
	df->lineIDX = 0;
	for (uint32_t i = 0; i < DELAY_MAX_LEN; i++) {
		df->line[i] = 0.0f;
	}

	/* Clear Output */
	df->output = 0.0f;

}


/* Calculate Delay Output */
float Delay_update(Delay_t *df, float input) {

	/* Get Current Delay Line Output */
	float delayLineOutput = df->line[df->lineIDX];

	/* Compute Current Delay Line Input */
	float delayLineInput = (input + (df->feedback * delayLineOutput));

	/* Store in Delay Line Circular Buffer */
	df->line[df->lineIDX] = delayLineInput;

	/* Increment Delay Line Index */
	df->lineIDX++;
	if (df->lineIDX >= DELAY_MAX_LEN) {
		df->lineIDX = 0;
	}

	/* Mix Dry and Wet Signals to Compute Output */
	df->output = (((1.0f - df->mix) * input) + (df->mix * delayLineOutput));

	/* Limit Output */
	if (df->output > 1.0f) {
		df->output = 1.0f;
	} else if (df->output < -1.0f) {
		df->output = -1.0f;
	}

	/* Return Current Output */
	return df->output;

}


/* Set Line Length */
void Delay_setLength(Delay_t *df, float delayTime_ms, float sampleRate_Hz) {

	/* Adjust Delay Line to Delay Time Setting */
	df->lineLen = (uint32_t) (0.001f * delayTime_ms * sampleRate_Hz);

	/* Handle Threshold Case */
	if (df->lineLen > DELAY_MAX_LEN) {
		df->lineLen = DELAY_MAX_LEN;
	}

}
