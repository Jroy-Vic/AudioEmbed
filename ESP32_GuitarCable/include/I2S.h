#ifndef __I2S_H
#define __I2S_H


#include <Arduino.h>
#include "driver/i2s.h"

/* MACROS */
// DMA HANDLERS
#define I2S_SAMPLE_RATE 44150
#define I2S_DMA_BUFF_LEN 100              // Number of Samples, not Bytes
#define BYTES_READ (I2S_DMA_BUFF_LEN * sizeof(uint16_t))
#define MAX_PENDING_PACKETS 150        // Number of Packets
#define MAX_DMA_PACKETS 600           // Number of Packets


/* External Variables */
extern uint16_t *DMA_ping_buff;
extern uint16_t *DMA_pong_buff;

/* Functions */
/* Initialize I2S Driver for Transmitting and Receiving */
uint8_t setupI2S(const uint8_t Transmitter_pins[]);


#endif