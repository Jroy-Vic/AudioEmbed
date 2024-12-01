/*
 * DAC.c
 *
 *  Created on: Jul 18, 2024
 *      Author: vicer
 */

#include "main.h"
#include "DAC.h"

/* Initialize and Configure DAC Peripheral */
/* Pins Used:
 * GPIOA - PA4, PA5, PA6, PA7
 */
//void DAC_init() {
//	/* Enable Clock Register for SPI1 */
//	  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
//	  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
//
//	/* Initialize GPIO for SPI1 for DAC */
//	  GPIOA->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
//	  GPIOA->MODER |= (GPIO_MODER_MODE4_1 | GPIO_MODER_MODE5_1 |
//			  	  	  	  GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1); // Alt. Func. (10)
//	  GPIOA->OTYPER &= ~(GPIO_OTYPER_OT4 | GPIO_OTYPER_OT5 | GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7); // PP (0)
//	  GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED4 | GPIO_OSPEEDR_OSPEED5 |
//			  	  	  	  GPIO_OSPEEDR_OSPEED6 | GPIO_OSPEEDR_OSPEED7);	// Very High-Speed (11)
//	  GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5 | GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7); // No PU/PD (00) for PA4,6
//	  GPIOA->PUPDR |= (GPIO_PUPDR_PUPD5_0 | GPIO_PUPDR_PUPD7_0); // PU (01) for PA5,7
//	  GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL4 | GPIO_AFRL_AFSEL5 | GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);
//	  GPIOA->AFR[0] |= ((GPIO_AFRL_AFSEL4_0 | GPIO_AFRL_AFSEL4_2) | (GPIO_AFRL_AFSEL5_0 | GPIO_AFRL_AFSEL5_2) |
//			  	  	  (GPIO_AFRL_AFSEL6_0 | GPIO_AFRL_AFSEL6_2) | (GPIO_AFRL_AFSEL7_0 | GPIO_AFRL_AFSEL7_2)); // AF5 (0101)
//
//	/* Initialize SPI Control Register */
//	  SPI1->CR1 &= ~SPI_CR1_BR;		// Baud Rate set to CLK/1 (000)
//	  SPI1->CR2 &= ~(SPI_CR2_DS);
//	  SPI1->CR2 |= (SPI_CR2_DS_3 | SPI_CR2_DS_1 | SPI_CR2_DS_0); // Data Size to 12 Bits (1011)
//	  SPI1->CR1 &= ~SPI_CR1_CPOL;	// Clock Polarity (0)
//	  SPI1->CR1 &= ~SPI_CR1_CPHA;	// Clock Phase (0)
//	  SPI1->CR1 |= SPI_CR1_MSTR;	// Enable Master Mode
//	  SPI1->CR2 |= SPI_CR2_NSSP;	// Set Pulse for CS
//	  SPI1->CR2 |= SPI_CR2_SSOE;	// Enable CS Output
//	  SPI1->CR1 |= SPI_CR1_SPE;		// Enable SPI Peripheral
//}
//
//
///* Send Data to the DAC (Includes DAC Configuration Bits to Data Input) */
//void DAC_Write(uint16_t LUT_volt) {
//	/* Wait for Tx to be empty */
//	//while (!(SPI1->SR & SPI_SR_TXE));
//
//	/* Write 12 Bits to DAC */
//	/* Add Configuration Bits (11) to Bits 13 and 14 of the Input */
//	LUT_volt &= BITMASK;
//	LUT_volt |= DAC_CAL << DAC_SHIFT;
//	/* Shift Data into Data Register */
//	SPI1->DR = LUT_volt;
//
//	/* Wait for Tx to be empty again and non-busy bus */
//	//while ((!(SPI1->SR & SPI_SR_TXE)) && (SPI1->SR & SPI_SR_BSY));
//}


