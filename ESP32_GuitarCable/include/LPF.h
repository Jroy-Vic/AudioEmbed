#ifndef __LPF_H_
#define __LPF_H_


#include <Arduino.h>


/* MACROS */
#define FIR_TAPS 5

/* Functions */
// Create Multi-Tap FIR Filter
uint8_t apply_fir_filter(uint8_t new_sample);


#endif