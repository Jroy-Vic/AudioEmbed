/*
 * Keypad.c
 *
 *  Created on: Dec 2, 2024
 *      Author: vicer
 */

#include "main.h"
#include "Keypad.h"

/* Initialize Keypad Peripheral:
 * PB1, PB13-15: Input GPIO
 * PB11-12, PA11-12: Output GPIO */
void Keypad_init(void) {

	/* Configure GPIO Clocks */
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN);

	/* Set Inputs (00) and Outputs (01) */
	GPIOA->MODER &= ~(GPIO_MODER_MODE11 | GPIO_MODER_MODE12);
	GPIOA->MODER |= (GPIO_MODER_MODE11_0 | GPIO_MODER_MODE12_0);
	GPIOB->MODER &= ~(GPIO_MODER_MODE1 | GPIO_MODER_MODE11 |
					GPIO_MODER_MODE12 | GPIO_MODER_MODE13 |
					GPIO_MODER_MODE14 | GPIO_MODER_MODE15);
	GPIOB->MODER |= (GPIO_MODER_MODE11_0 | GPIO_MODER_MODE12_0);
	/* Set OTYPER to PP (0) */
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT11 | GPIO_OTYPER_OT12);
	GPIOB->OTYPER &= ~(GPIO_OTYPER_OT11 | GPIO_OTYPER_OT12);
	/* Set OSPEEDR to Low (00) */
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED11 | GPIO_OSPEEDR_OSPEED12);
	GPIOB->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED11 | GPIO_OSPEEDR_OSPEED12);
	/* Set PUDPR to PD (10) */
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD11 | GPIO_PUPDR_PUPD12);
	GPIOA->PUPDR |= (GPIO_PUPDR_PUPD11_1 | GPIO_PUPDR_PUPD12_1);
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD11 | GPIO_PUPDR_PUPD12);
	GPIOB->PUPDR |= (GPIO_PUPDR_PUPD11_1 | GPIO_PUPDR_PUPD12_1);
}

/* Reset All Columns to Polling State */
void Keypad_reset(void) {

	/* Output High Signal */
	GPIOA->ODR |= (GPIO_ODR_OD11 | GPIO_ODR_OD12);
	GPIOB->ODR |= (GPIO_ODR_OD11 | GPIO_ODR_OD12);
}

/* Output Keypad Value */
int8_t Keypad_read(void) {

}
