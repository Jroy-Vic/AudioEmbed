#ifndef __I2S_H
#define __I2S_H


#include <Arduino.h>
#include "driver/i2s.h"

/* MACROS */
#define I2S_SAMPLE_RATE 44100
#define I2S_DMA_BUFF_LEN 20          // Number of Samples, not Bytes
#define PREBUFF_PACKET_LEN 5         // Number of Packets
#define PREBUFF_OVERRUN_REFRESH 8    // Number of Packets to Reset

/* External Variables */
extern uint16_t DMA_ping_buff[I2S_DMA_BUFF_LEN];
extern uint16_t DMA_pong_buff[I2S_DMA_BUFF_LEN];

/* Functions */
/* Initialize I2S Driver for Transmitting and Receiving */
void setupI2S(const uint8_t Transmitter_pins[]);


#endif