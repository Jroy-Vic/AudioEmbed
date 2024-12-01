/*
 * LED_Debug.c
 *
 *  Created on: Sep 2, 2024
 *      Author: vicer
 */


#include "main.h"
#include "LED_Debug.h"


/* Initialize GPIO Pin for LED Debug
 * Pins Used: PC2, PC3
 * */
void LED_Debug_init() {
	/* Enable GPIOC Clock */
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

	/* Configure PC0 to be PP, PD, and Low-Speed */
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT3 |GPIO_OTYPER_OT2);
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD3 | GPIO_PUPDR_PUPD2);
	GPIOC->PUPDR |= (GPIO_PUPDR_PUPD3_1 | GPIO_PUPDR_PUPD2_1);
	GPIOC->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED3 | GPIO_OSPEEDR_OSPEED2);

	/* Set GPIO to Output Mode (01) */
	GPIOC->MODER &= ~GPIO_MODER_MODE3;
	GPIOC->MODER |= GPIO_MODER_MODE3_0;
	GPIOC->MODER &= ~GPIO_MODER_MODE2;
	GPIOC->MODER |= GPIO_MODER_MODE2_0;

	/* Set GPIO Output to 0 */
	GPIOC->ODR &= ~GPIO_ODR_OD2;
	GPIOC->ODR &= ~GPIO_ODR_OD3;
}

/* Toggle LED1 */
void LED_Debug_1_toggle() {
	GPIOC->ODR ^= GPIO_ODR_OD3;
}

/* Toggle LED2 */
void LED_Debug_2_toggle() {
	GPIOC->ODR ^= GPIO_ODR_OD2;
}
