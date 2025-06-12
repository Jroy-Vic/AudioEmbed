#ifndef __FIFO_H_
#define __FIFO_H_


#include <Arduino.h>
#include "I2S.h"


/* MACROS */
#define PREBUFF_LEN 512

/* External Variables */
extern uint8_t Task_buff[PREBUFF_LEN];
extern volatile uint16_t head;
extern volatile uint16_t tail;


/* Functions */
// Check if Buffer is Full
uint8_t isFull(void);

// Check if Buffer is Empty
uint8_t isEmpty(void);

// Push Sample into FIFO Queue
uint8_t push_Sample(uint8_t sample);

// Pop Sample out of FIFO Queue
uint8_t pop_Sample(uint8_t *sample);

// Check how Many Samples are Pre-Buffered
uint16_t sampleCount();


#endif