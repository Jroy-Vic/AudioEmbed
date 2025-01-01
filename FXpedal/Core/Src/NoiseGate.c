/*
 * NoiseGate.c
 *
 *  Created on: Dec 20, 2024
 *      Author: vicer
 */


#include "main.h"
#include "NoiseGate.h"


/* Initialize Noise Gate Filter */
void NoiseGate_init(NoiseGateFilt_t *ngf, float threshold, float attackTime,
					float releaseTime, float holdTime, float sampleRate) {

	/* Create Noise Gate Filter (Convert Hold Time from Seconds to Milliseconds) */
	NoiseGate_setThreshold(ngf, threshold);
	ngf->holdTime = (0.001f * holdTime);

	/* Calculate Attack and Release Coefficients for LPF */
	NoiseGate_setAttackReleaseTime(ngf, attackTime, releaseTime, sampleRate);

	/* Save Sample Time */
	ngf->sampleTime = (1.0f / sampleRate);

	/* Reset Counters */
	ngf->attackCnt = 0.0f;
	ngf->releaseCnt = 0.0f;

	/* Reset Smoothed Gain Value */
	ngf->smoothedGain = 0.0f;

}


/* Update Noise Gate Filter */
float NoiseGate_update(NoiseGateFilt_t *ngf, float input) {

	/* Calculate Magnitude of Input */
	float magInput = fabsf(input);

	/* Handle Static Gain; Restrict Lower Threshold Value */
	float gain = 1.0f;
	if (magInput < ngf->threshold) {
		gain = 0.0f;
	}

	/* Smoothen the Gain in Release or Attack Stage */
	/* Attack Stage */
	if (gain <= ngf->smoothedGain) {

		/* Handle Attack Time */
		if (ngf->attackCnt > ngf->holdTime) {

			/* Apply Attack LPF; Decrease Gain to Reduce Output
			 * x[n] = (C * x[n]) + ((1.0f - C) * gain))
			*/
			ngf->smoothedGain = (ngf->attackCoeff * ngf->smoothedGain)
					+ ((1.0f - ngf->attackCoeff) * gain);
		}
		/* Increment Counter if Hold Time Has Not Yet Elapsed */
		else {
			ngf->attackCnt += ngf->sampleTime;
		}
	}

	/* Release Stage */
	else if (gain > ngf->smoothedGain) {

		/* Apply Release LPF; Increase Gain to Amplify Output
		 * x[n] = (C * x[n]) + ((1.0f - C) * gain))
		*/
		ngf->smoothedGain = (ngf->releaseCoeff * ngf->smoothedGain)
				+ ((1.0f - ngf->releaseCoeff) * gain);

		/* Reset Attack Counter Once in Release Stage */
		ngf->attackCnt = 0.0f;
	}

	/* Output Modified Input */
	return (input * ngf->smoothedGain);
}


/* Set Threshold Value */
void NoiseGate_setThreshold(NoiseGateFilt_t *ngf, float threshold) {

	/* Limit Threshold Value to Unity Range (0.0f - 1.0f) */
	if (threshold > 1.0f) {
		threshold = 1.0f;
	} else if (threshold < 0.0f) {
		threshold = 0.0f;
	}

	/* Store Threshold Value */
	ngf->threshold = threshold;

}


/* Set Attack and Release Time */
void NoiseGate_setAttackReleaseTime(NoiseGateFilt_t *ngf, float attackTime,
					float releaseTime, float sampleTime) {

	/* Calculate Attack and Release Time Coefficients
	 * coeff = exp((-2.2 * R * C) / (sampleRate * attack/releaseTime))
	 *** Time Constant is in respect to Milliseconds ***
	 */
	ngf->attackCoeff = expf(-2197.22457734f / (sampleTime * attackTime));
	ngf->releaseCoeff = expf(-2197.22457734f / (sampleTime * releaseTime));

}

