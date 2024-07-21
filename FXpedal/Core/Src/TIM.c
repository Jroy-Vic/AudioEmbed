/*
 * TIM.c
 *
 *  Created on: Jul 18, 2024
 *      Author: vicer
 */


#include "TIM.h"
#include "main.h"

/* Extern Variables */
uint8_t Output_Flag;


/* Initialize Timer (Set to 24MHz, Prescaled to 500kHz) */
void TIM_init() {
  /* Enabling clock for TIM2 (24MHz) */
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

  TIM2->CR1 &= ~TIM_CR1_CEN;	// Temporarily turn off Timer
  TIM2->PSC = TIM2_PRESCALER;	// Scale Clock down to 500kHz to Save Power
  TIM2->ARR = TIM2_ARR;			// Creates 10kHz Interrupt

  /* Set TIM2 interrupts to be highest priority */
  NVIC->IP[TIM2_IRQn] = TIM2_PRIORITY;
  /* Enable NVIC to handle TIM2 interrupts */
  NVIC->ISER[TIM2_NVIC] = (1 << (TIM2_IRQn & 0x1F));
  __enable_irq();

  /* Set Timer Conditions and Enable */
  TIM2->CR1 &= ~TIM_CR1_UDIS;	// Enable UEVs
  TIM2->DIER |= (TIM_DIER_UIE);	// Enable hardware interrupt
  TIM2->CR1 |= TIM_CR1_CEN;	// Enable timer
  TIM2->EGR |= TIM_EGR_UG;	// Force Update Event to reset timer
  TIM2->EGR |= ~TIM_EGR_UG;	// Toggle off Force Update Event
}


/* Force Timer Reset */
void TIM_reset() {
	TIM2->EGR |= TIM_EGR_UG;	// Force Update Event to reset timer
	TIM2->EGR |= ~TIM_EGR_UG;	// Toggle off Force Update Event

	/* Clear Flags */
	Output_Flag = CLEAR;
}


/* Disable TIM2 */
void TIM_disable() {
	TIM2->CR1 &= ~TIM_CR1_CEN;	// Temporarily turn off Timer
}


/* Re-Enable TIM2 */
void TIM_enable() {
	TIM2->CR1 |= TIM_CR1_CEN;	// Enable timer
	TIM_reset();
}


/* TIM2 Interrupt Handler */
/* Creates a 10kHz Interrupt to Begin ADC Conversion */
void TIM2_IRQHandler() {
	/* If ARR is Reached, Toggle Output_Flag */
	Output_Flag = SET;

	/* Clear UIF */
	TIM2->SR &= ~TIM_SR_UIF;
}
