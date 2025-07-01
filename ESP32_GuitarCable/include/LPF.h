#ifndef __LPF_H_
#define __LPF_H_


#include <Arduino.h>


/* MACROS */
#define FIR_TAPS 8

/* Functions */
// Create Multi-Tap FIR Filter
uint16_t apply_fir_filter(uint16_t new_sample);


#endif