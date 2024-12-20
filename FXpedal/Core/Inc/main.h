/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "LPF.h"
#include "DelayFilter.h"

/* MACROS */
#define SET 0x1
#define CLEAR 0x0
#define BUFFER_SIZE 6
#define FFT_BUFFER_SIZE 2048
#define FLOAT_TO_INT16(x) ((int16_t)((x) * 32768.0f))
#define INT16_TO_FLOAT(x) ((float) (x) / 32768.0f)
#define FFT 0x0
#define IFFT 0x1
#define GAIN 1.0f
#define CORNER_FREQ 5000.0f
#define SAMP_FREQ 48000.0f
#define DELAY_SIZE 500
#define DELAY_CUTOFF (0.1f)
#define _AMP(x) ( x / 2 )


/* Functions */
/* Process Stored Data in Buffer */
void processData(LPF_t *lpf, DelayFilter_t *dft);


/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
