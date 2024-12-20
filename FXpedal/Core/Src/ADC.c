/*
 * ADC.c
 *
 *  Created on: Jul 18, 2024
 *      Author: vicer
 */


#include "main.h"
#include "ADC.h"

/* Extern Variables */
uint16_t GtrSamp_DigVal;
uint8_t Input_Flag;


/* Initialize ADC1 Peripheral */
/* Pins Used:
 * GPIOC - PC0
 */
/* Running the ADC with a 24MHz Clock
 * Single Conversion, initiated with SC Bit
 * Using sampler; Hold timer with sample time of 2.5 clocks
 * 12-bit Conversion using 3.3V Reference
 * Configure analog input pin
 */
void ADC_init() {
	/* Configure Analog Input Pin for Channel 1 (PC0) */
	/* Enable GPIOC Clock */
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	/* Configure PC0 to be PP, No PUPDR, and High-Speed */
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT0;
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD0;
	GPIOC->OSPEEDR |= GPIO_OSPEEDR_OSPEED0;
	/* Set GPIO to Analog Mode for ADC (11) */
	GPIOC->MODER |= GPIO_MODER_MODE0;
	/* Connect Analog Switch to the ADC Input (1) */
	GPIOC->ASCR |= GPIO_ASCR_ASC0;

	/* Enable 48MHz ADC Clock and set to Synchronous Mode */
	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;
	ADC123_COMMON->CCR |= ADC_CCR_CKMODE;

	/* Power Up ADC (Turn off Deep Power Down Mode) */
	ADC1->CR &= ~ADC_CR_DEEPPWD;
	/* Enable Voltage Regulator */
	ADC1->CR |= ADC_CR_ADVREGEN;
	/* Wait 20 us to ensure regulator startup time has elapsed */
	for (uint8_t i = 0; i < TWENTYU_DELAY; i++);
	while (!(ADC1->CR & ADC_CR_ADVREGEN));

	/* Ensure ADC is Disabled*/
	ADC1->CR &= ~ADC_CR_ADEN;

	/* Select Input Mode for Calibration (Single-ended Input [0]) */
	ADC1->CR &= ~ADC_CR_ADCALDIF;
	/* Calibrate ADC and Wait Until Complete (ADCAL returns to 0) */
	ADC1->CR |= ADC_CR_ADCAL;
	while (ADC1->CR & ADC_CR_ADCAL);

	/* Set Channel 1 (PC0) as Single-ended Mode (0) */
	ADC1->DIFSEL &= ~ADC_DIFSEL_DIFSEL_1;

	/* Configure ADC1 (Clear ADSTART Initially) */
	ADC1->CR &= ~ADC_CR_ADSTART;
	/* Set to Continuous Conversion Mode (1) */
	ADC1->CFGR |= ADC_CFGR_CONT;
	/* Set to Right-Aligned Data (0) */
	ADC1->CFGR &= ~ADC_CFGR_ALIGN;
	/* Set to 12-bit Resolution (00) */
	ADC1->CFGR &= ~ADC_CFGR_RES;
	/* Set Channel 1 as a Single Regular Sequence (1) */
	ADC1->SQR1 |= ADC_SQR1_SQ1_0;
	/* Set Sample Time to 92.5 Clocks to Channel 1 (101) */
	ADC1->SMPR1 &= ~ADC_SMPR1_SMP1;
	ADC1->SMPR1 |= (0b101 << ADC_SMPR1_SMP1_Pos);

	/* Configure ADC1 to Receive Trigger from TIM6_TRGO (1101) */
	ADC1->CFGR &= ~ADC_CFGR_EXTSEL;
	ADC1->CFGR |= (0b1101 << ADC_CFGR_EXTSEL_Pos);
	/* Allow Conversions to be Set by Rising Edge of TIM6_TRGO (01) */
	ADC1->CFGR &= ~ADC_CFGR_EXTEN;
	ADC1->CFGR |= ADC_CFGR_EXTEN_0;

	/* Clear ADC Ready Flag (Write 1 to Bit), Then Enable ADC */
	ADC1->ISR |= ADC_ISR_ADRDY;
	ADC1->CR &= ~ADC_CR_ADDIS;
	ADC1->CR |= ADC_CR_ADEN;
	/* Hardware Sets ADRDY Flag; Wait for Bit to be Set */
	while(!(ADC1->ISR & ADC_ISR_ADRDY));

	/* Enable ADC DMA Transfer */
	ADC1->CFGR |= ADC_CFGR_DMAEN;
	ADC1->CFGR |= ADC_CFGR_DMACFG;
}


/* Begin Continuous Conversion
 * Sets ADSTART to Begin a New Conversion Sample
 * ADSTART is cleared by hardware when initiated
 */
void ADC_collect(void) {
	/* Begin ADC Conversion */
	ADC1->CR |= ADC_CR_ADSTART;
}

