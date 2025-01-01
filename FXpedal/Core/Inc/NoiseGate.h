/*
 * NoiseGate.h
 *
 *  Created on: Dec 20, 2024
 *      Author: vicer
 */

#ifndef INC_NOISEGATE_H_
#define INC_NOISEGATE_H_

#include <math.h>

#define NGF_THRESHOLD (0.1f)
#define NGF_ATTACKTIME (5.0f)
#define NGF_RELEASETIME (2.0f)
#define NGF_HOLDTIME (5.0f)

typedef struct {
	/* Threshold Margin for Noise Suppression */
	float threshold;

	/* Hold and Sample Time (milliseconds) */
	float holdTime, sampleTime;

	/* Coefficients for LPF */
	float attackCoeff, releaseCoeff;

	/* Attack and Release Counters */
	float attackCnt, releaseCnt;

	/* Smoothed Gain */
	float smoothedGain;
} NoiseGateFilt_t;


/* Initialize Noise Gate Filter */
void NoiseGate_init(NoiseGateFilt_t *ngf, float threshold, float attackTime,
					float releaseTime, float holdTime, float sampleRate);

/* Update Noise Gate Filter */
float NoiseGate_update(NoiseGateFilt_t *ngf, float input);

/* Set Threshold Value */
void NoiseGate_setThreshold(NoiseGateFilt_t *ngf, float threshold);

/* Set Attack and Release Time */
void NoiseGate_setAttackReleaseTime(NoiseGateFilt_t *ngf, float attackTime,
					float releaseTime, float sampleTime);


#endif /* INC_NOISEGATE_H_ */
