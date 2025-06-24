#ifndef __I2S_H
#define __I2S_H


#include <Arduino.h>
#include "driver/i2s.h"

/* MACROS */
// DMA HANDLERS
#define I2S_SAMPLE_RATE 19400
#define I2S_DMA_BUFF_LEN 8           // Number of Samples, not Bytes
#define BYTES_READ (I2S_DMA_BUFF_LEN * sizeof(uint16_t))
#define MAX_PENDING_PACKETS 50        // Number of Packets
#define MAX_DMA_PACKETS 150           // Number of Packets


/* External Variables */
extern uint16_t DMA_ping_buff[I2S_DMA_BUFF_LEN];
extern uint16_t DMA_pong_buff[I2S_DMA_BUFF_LEN];

/* Functions */
/* Initialize I2S Driver for Transmitting and Receiving */
void setupI2S(const uint8_t Transmitter_pins[]);


#endif