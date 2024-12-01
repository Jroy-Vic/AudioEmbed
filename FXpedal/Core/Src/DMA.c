/*
 * DMA.c
 *
 *  Created on: Jul 27, 2024
 *      Author: vicer
 */

#include "main.h"
#include "DMA.h"
#include "LED_Debug.h"

/* Initialize DMA Peripheral-to-Memory
 * / Memory-to-Peripheral Transfer */
/* ADC to DAC DMA Transfer, using Circular Buffer
 * ADC1: Channel 1
 * DAC1: Channel 1*/
void DMA_init(int16_t *inBuff, int16_t *outBuff, uint16_t buff_size) {
	/* Initialize Clock for DMA1/2 */
	RCC->AHB1ENR |= (RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMA2EN);

	/* Disable DMA1 Channel 1/DMA2 Channel 4 to Configure */
	DMA1_Channel1->CCR &= ~DMA_CCR_EN;
	DMA2_Channel4->CCR &= ~DMA_CCR_EN;

	/* Set Peripheral Register Address (ADC1) in DMA1_CPAR1 */
	DMA1_Channel1->CPAR = (uint32_t) &(ADC1->DR);
	/* Set Peripheral Register Address (DAC1) in DMA2_CPAR4 */
	DMA2_Channel4->CPAR = (uint32_t) &(DAC1->DHR12R1);

	/* Set Memory Address (inBuff) in DMA1_CMAR1 */
	DMA1_Channel1->CMAR = (uint32_t) inBuff;
	/* Set Memory Address (outBuff) in DMA2_CMAR4 */
	DMA2_Channel4->CMAR = (uint32_t) outBuff;

	/* Configure the Number of Data to Transfer in DMA_CNDTR1/4 */
	DMA1_Channel1->CNDTR = buff_size;
	DMA2_Channel4->CNDTR = buff_size;

	/* Configure Channel Select for DMA1/2 for ADC1/DAC_CH1 (C1S = 0x0/C4S = 0x3) */
	DMA1_CSELR->CSELR &= ~DMA_CSELR_C1S;
	DMA2_CSELR->CSELR &= ~DMA_CSELR_C4S;
	DMA2_CSELR->CSELR |= (0x3 << DMA_CSELR_C4S_Pos);

	/* Configure DMA1_CCR1
	 * Priority: Very High (11)
	 * Data Transfer Direction: MEM2MEM - Disabled (0), DIR - Read from Peripheral (0)
	 * Circular Mode: CIRC - Enabled (1)
	 * Peripheral and Memory Incremented Mode: MINC - Enabled (1), PINC - Disabled (0)
	 * Peripheral and Memory Data Size: MSIZE - 16 Bits (01), PSIZE - 16 Bits (01)
	 * Interrupt Enable: TCIE - Enabled (1), HTIE - Enabled (1)
	 * */
	DMA1_Channel1->CCR |= DMA_CCR_PL;
	DMA1_Channel1->CCR &= ~(DMA_CCR_MEM2MEM | DMA_CCR_DIR);
	DMA1_Channel1->CCR |= DMA_CCR_CIRC;
	DMA1_Channel1->CCR |= DMA_CCR_MINC;
	DMA1_Channel1->CCR &= ~DMA_CCR_PINC;
	DMA1_Channel1->CCR &= ~(DMA_CCR_MSIZE | DMA_CCR_PSIZE);
	DMA1_Channel1->CCR |= (DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0);
	DMA1_Channel1->CCR |= (DMA_CCR_TCIE | DMA_CCR_HTIE);

	/* Configure DMA2_CCR4
		 * Data Transfer Direction: MEM2MEM - Disabled (0), DIR - Read from Memory (1)
		 * Circular Mode: CIRC - Enabled (1)
		 * Peripheral and Memory Incremented Mode: MINC - Enabled (1), PINC - Disabled (0)
		 * Peripheral and Memory Data Size: MSIZE - 16 Bits (01), PSIZE - 16 Bits (01)
		 * Interrupt Enable: TCIE - Enabled (1), HTIE - Enabled (1)
		 * */
	DMA2_Channel4->CCR |= DMA_CCR_PL;
	DMA2_Channel4->CCR &= ~DMA_CCR_MEM2MEM;
	DMA2_Channel4->CCR |= DMA_CCR_DIR;
	DMA2_Channel4->CCR |= DMA_CCR_CIRC;
	DMA2_Channel4->CCR |= DMA_CCR_MINC;
	DMA2_Channel4->CCR &= ~DMA_CCR_PINC;
	DMA2_Channel4->CCR &= ~(DMA_CCR_MSIZE | DMA_CCR_PSIZE);
	DMA2_Channel4->CCR |= (DMA_CCR_MSIZE_0 | DMA_CCR_PSIZE_0);
	DMA2_Channel4->CCR |= (DMA_CCR_TCIE | DMA_CCR_HTIE);

	/* Enable NVIC for Interrupts */
	NVIC_SetPriority(DMA1_Channel1_IRQn, DMA1_CH1_NVIC_PRIORITY);
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	NVIC_SetPriority(DMA2_Channel4_IRQn, DMA2_CH4_NVIC_PRIORITY);
	NVIC_EnableIRQ(DMA2_Channel4_IRQn);
	__enable_irq();

	/* Clear Any Pending Interrupts Before Enabling DMA Channel */
	DMA1->IFCR |= DMA_IFCR_CGIF1;
	DMA1->IFCR |= DMA_IFCR_CHTIF1;
	DMA1->IFCR |= DMA_IFCR_CTCIF1;
	DMA2->IFCR |= DMA_IFCR_CGIF4;
	DMA2->IFCR |= DMA_IFCR_CHTIF4;
	DMA2->IFCR |= DMA_IFCR_CTCIF4;
}


/* Enable DMA Channels */
void DMA_enable() {
	/* Activate Channel 1 and 4 */
	DMA1_Channel1->CCR |= DMA_CCR_EN;
	DMA2_Channel4->CCR |= DMA_CCR_EN;
}


/* Interrupt Handlers */

/* Interrupt Handler for DMA1 Channel 1 (Input Data) */
void DMA1_Channel1_IRQHandler(void) {
	/* If Half of the Buffer is Filled (Ping) */
	if (DMA1->ISR & DMA_ISR_HTIF1) {
		/* Reset Buffer Pointer to First Half to Process */
		inBuffPtr = &inBuff[0];
		outBuffPtr = &outBuff[0];

		/* Set Data Ready Flag */
		Data_Ready_Flag = SET;

		/* Clear Interrupt Flag */
		DMA1->IFCR |= DMA_IFCR_CHTIF1;
	}
	/* If the Entire Buffer is Filled (Pong) */
	else if (DMA1->ISR & DMA_ISR_TCIF1) {
		/* Set Buffer Pointer to Second Half to Process */
		inBuffPtr = &inBuff[(BUFFER_SIZE / 2)];
		outBuffPtr = &outBuff[(BUFFER_SIZE / 2)];

		/* Set Data Ready Flag */
		Data_Ready_Flag = SET;

		/* Clear Interrupt Flag */
		DMA1->IFCR |= DMA_IFCR_CTCIF1;
	}


}

/* Interrupt Handler for DMA2 Channel 4 (Output Data) */
void DMA2_Channel4_IRQHandler(void) {
	/* If Half of the Buffer is Filled (Ping) */
	if (DMA2->ISR & DMA_ISR_HTIF4) {
		/* Debug: Toggle LED */
		LED_Debug_2_toggle();

		/* Clear Interrupt Flag */
		DMA2->IFCR |= DMA_IFCR_CHTIF4;
	}
	/* If the Entire Buffer is Filled (Pong) */
	else if (DMA2->ISR & DMA_ISR_TCIF4) {
		/* Debug: Toggle LED */
		LED_Debug_2_toggle();

		/* Clear Interrupt Flag */
		DMA2->IFCR |= DMA_IFCR_CTCIF4;
	}
}
