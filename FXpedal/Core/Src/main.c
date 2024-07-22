#include "main.h"
#include "TIM.h"
#include "DAC.h"
#include "ADC.h"
#include "LPF.h"

/* FFT */
#define ARM_MATH_CM4
#include "arm_math.h"

/* MACROS */
#define BUFFER_SIZE 0x200
#define FLOAT_TO_UINT16(f) ((uint16_t)((f) * 65535.0f + 0.5f))
#define UINT16_TO_FLOAT(u) ((float)(u) / 65535.0f)
#define FFT 0x0
#define IFFT 0x1
#define GAIN 0x2
#define CORNER_FREQ 5000
#define SAMP_FREQ 48000

/* Global Variables */
extern uint16_t GtrSamp_DigVal;
extern uint8_t Input_Flag;
extern uint8_t Output_Flag;


/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize ADC and DAC for I/O */
  ADC_init();
  DAC_init();

  /* Create Arrays and Variables for Circular Buffering */
  float inBuff[BUFFER_SIZE], outBuff[BUFFER_SIZE];
  uint16_t buffIDX = 0x0;
  /* Clear outBuff to Prevent any Initial Unwanted Feedback */
  for (uint16_t i = 0x0; i < BUFFER_SIZE; i++) {
	  outBuff[i] = 0.0f;
  }

  /* Initialize FFT Handler */
  arm_rfft_fast_instance_f32 fftHandler;
  arm_rfft_fast_init_f32(&fftHandler, BUFFER_SIZE);

  /* Initialize First-Order Low Pass Filter */
  LPF_t lpfHandler;
  LPF_init(*lpfHandler, corner_freq, samp_freq);

  /* Initialize TIM2 to Begin Sample Collection */
  TIM_init();


  while (1)
  {
	  /* Check if ARR has been Reached: Output and Collect New Sample */
	  if (Output_Flag) {
//		  /* Output Value from Output Buffer to DAC (Converted to uint16_t from float) */
//		  DAC_Write((uint16_t) (FLOAT_TO_UINT16(outBuff[buffIDX]) * GAIN));

		  /* Begin ADC Conversion to Collect Guitar Sample */
		  ADC_collect();

		  /* Clear ARR Flag */
		  Output_Flag = CLEAR;
	  }

	  /* Process Collected Sample */
	  if (Input_Flag) {
//		  /* Store Sample Value from ADC into Input Buffer (Converted to float from uint16_t) */
//		  inBuff[buffIDX] = (float) (UINT16_TO_FLOAT(GtrSamp_DigVal) * GAIN);
//
//		  /* Increment Input Buffer Counter; Apply FFT Once Full */
//		  buffIDX++;
//		  if (buffIDX == BUFFER_SIZE) {
//			  /* Reset Input Buffer Counter */
//			  buffIDX = 0x0;
//
//			  /* Apply FFT and Store to Output Buffer */
//			  arm_rfft_fast_f32(&fftHandler, inBuff, outBuff, FFT);
//
//			  /* Apply Effects */
//			  //
//
//			  /* Apply iFFT and Replace Output Buffer */
//			  arm_rfft_fast_f32(&fftHandler, outBuff, outBuff, IFFT);
//		  }

		  DAC_Write((GtrSamp_DigVal * GAIN));
		  /* Clear ADC Flag */
		  Input_Flag = CLEAR;
	  }
  }

}



























/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_9;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
