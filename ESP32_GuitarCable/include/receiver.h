#ifndef __RECEIVER_H
#define __RECEIVER_H


#include <Arduino.h>
#include "I2S.h"
#include "ESP_COMM.h"
#include "LPF.h"
#include "FIFO.h"
#include <Ticker.h>


/* MACROS */
#define DEBUG_PIN 23
#define RECEIVER_PINS {25}
#define DAC_TIMER_INTERVAL_US 23

/* External Variables */
extern const uint8_t Receiver_pins[];

/* Functions*/
void receiver_setup(void);
void receiver_loop(void);

// Handle Incoming Signal Data
void onReceive(const uint8_t *mac, const uint8_t *incomingData, int len);

// Output Filtered Sample through DAC at 44.1kHz
void IRAM_ATTR onDACClock(void);

#endif