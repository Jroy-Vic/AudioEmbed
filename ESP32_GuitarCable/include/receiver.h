#ifndef __RECEIVER_H
#define __RECEIVER_H


#include <Arduino.h>
#include "I2S.h"
#include "ESP_COMM.h"
#include "LPF.h"
#include "FIFO.h"
#include "MCP4921.h"
#include <Ticker.h>


/* MACROS */
#define DEBUG_PIN 4
#define DEBUG_PIN2 22
#define LED_PIN 2
#define RECEIVER_PINS {5, 18, 23}    // VSPI CS, SCK, SDI
// DAC TIMER HANDLERS
#define DAC_TIMER_INTERVAL_US 52
#define PREBUFF_PACKET_LEN 4         // Number of Packets
#define PREBUFF_OVERRUN_REFRESH 6    // Number of Packets to Reset

/* External Variables */
extern const uint8_t Receiver_pins[];

/* Functions*/
void receiver_setup(void);
void receiver_loop(void);

// Handle Incoming Signal Data
void onReceive(const uint8_t *mac, const uint8_t *incomingData, int byte_len);

// Output Filtered Sample through DAC at 44.1kHz
void IRAM_ATTR onDACClock(void);

#endif