#include <Arduino.h>
#include "LPF.h"


/* Global Variables */
uint16_t fir_buffer[FIR_TAPS] = {0};
uint16_t fir_idx = 0;


/* Functions */
// Create Multi-Tap FIR Filter
uint16_t apply_fir_filter(uint16_t new_sample) {
    fir_buffer[fir_idx] = new_sample;
    fir_idx = (fir_idx + 1) % FIR_TAPS;

    uint16_t sum = 0;
    for (uint8_t i = 0; i < FIR_TAPS; i++) {
        sum += fir_buffer[i];
    }

    return (uint16_t)(sum / FIR_TAPS);
}