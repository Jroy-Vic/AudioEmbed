/*
 * DAC1_CH1.c
 *
 *  Created on: Aug 31, 2024
 *      Author: vicer
 */

#include "DAC1_CH1.h"
#include "main.h"

/* Initialize and Configure DAC1 Peripheral */
/* Pins Used:
 * GPIOA - PA4
 */
void DAC_init() {
	/* Enable Clock Register for GPIOA and DAC1 */
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	RCC->APB1ENR1 |= RCC_APB1ENR1_DAC1EN;

	/* Initialize GPIOA for DAC1_CH1 */
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT4;		// OTYPE - PP (0)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD4;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD4_1; 	// PUPD - PD (10)
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED4;	// OSPEED - High-Speed (11)
	GPIOA->MODER |= GPIO_MODER_MODE4;		// Alt. Func. - Analog (11)
	GPIOA->ASCR &= ~GPIO_ASCR_ASC4;			// ASC - Output (0)

	/* Ensure DAC1_CH1 is Disabled */
	DAC1->CR &= ~DAC_CR_EN1;

	/* Enable DAC1_CH1 Output Buffer to Reduce Issues with Impedance Matching */
	DAC1->MCR &= ~DAC_MCR_MODE1;

	/* Enable TIM6_TRGO Trigger on Channel 1 (000) */
	DAC1->CR &= ~DAC_CR_TSEL1;

	/* Enable DAC1 Channel 1 */
	DAC1->CR |= DAC_CR_EN1;
	DAC1->CR |= DAC_CR_TEN1;

	/* Enable DAC1 Channel 1 DMA Transfer */
	DAC1->CR |= DAC_CR_DMAEN1;


}
