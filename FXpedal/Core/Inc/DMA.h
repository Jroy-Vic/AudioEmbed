/*
 * DMA.h
 *
 *  Created on: Jul 27, 2024
 *      Author: vicer
 */

#ifndef INC_DMA_H_
#define INC_DMA_H_

#include "main.h"

/* Macros */
#define DMA1_CH1_NVIC_PRIORITY 0x0
#define DMA1_CH1_NVIC (((uint32_t) DMA1_Channel1_IRQn) >> 5UL)
#define DMA2_CH4_NVIC_PRIORITY 0x1
#define DMA2_CH4_NVIC (((uint32_t) DMA1_Channel3_IRQn) >> 5UL)

///* Global Variables */
extern volatile uint8_t Data_Ready_Flag;
extern int16_t inBuff[BUFFER_SIZE], outBuff[BUFFER_SIZE];
extern volatile int16_t *inBuffPtr, *outBuffPtr;

/* Initialize DMA Peripheral-to-Memory
 * / Memory-to-Peripheral Transfer */
void DMA_init(int16_t *inBuff, int16_t *outBuff, uint16_t buff_size);

/* Enable DMA Channels */
void DMA_enable(void);

#endif /* INC_DMA_H_ */
