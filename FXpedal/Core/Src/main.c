#include "main.h"
#include "DAC1_CH1.h"
#include "TIM.h"
#include "DMA.h"
#include "ADC.h"
#include "LPF.h"
#include "HPF.h"
#include "DelayFilter.h"
#include "NoiseGate.h"
#include "Delay.h"
#include "LED_Debug.h"

/* FFT */
#define ARM_MATH_CM4
#include "arm_math.h"

/* Global Variables */
volatile uint8_t Data_Ready_Flag;
int16_t inBuff[BUFFER_SIZE], outBuff[BUFFER_SIZE];
volatile int16_t *inBuffPtr = inBuff, *outBuffPtr = outBuff;
//extern uint16_t GtrSamp_DigVal;
//extern uint8_t Input_Flag;
extern uint8_t Output_Flag;

const size_t SINE_SAMPLES = 32;
const uint16_t SINE_WAVE[] = {
  _AMP(2048), _AMP(2447), _AMP(2831), _AMP(3185),
  _AMP(3495), _AMP(3750), _AMP(3939), _AMP(4056),
  _AMP(4095), _AMP(4056), _AMP(3939), _AMP(3750),
  _AMP(3495), _AMP(3185), _AMP(2831), _AMP(2447),
  _AMP(2048), _AMP(1649), _AMP(1265), _AMP(911),
  _AMP(601),  _AMP(346),  _AMP(157),  _AMP(40),
  _AMP(0),    _AMP(40),   _AMP(157),  _AMP(346),
  _AMP(601),  _AMP(911),  _AMP(1265), _AMP(1649)
};

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize LED Debugger */
  LED_Debug_init();

  /* Initialize DMA Transfer */
  DMA_init(inBuff, outBuff, BUFFER_SIZE);

  /* Initialize TIM6_TRGO, ADC, and DAC for I/O */
  TIM_init();
  ADC_init();
  DAC_init();

  /* Enable DMA Stream */
  DMA_enable();

  /* Clear outBuff to Prevent any Initial Unwanted Feedback */
  for (uint16_t i = 0x0; i < BUFFER_SIZE; i++) {
	  inBuff[i] = 0;
	  outBuff[i] = 0;
  }

//  /* Initialize FFT Handler */
//  arm_rfft_fast_instance_f32 fftHandler;
//  arm_rfft_fast_init_f32(&fftHandler, FFT_BUFFER_SIZE);
//
  /* Initialize First-Order Low Pass Filter */
  LPF_t lpfHandler;
  LPF_init(&lpfHandler, LPF_CORNER_FREQ, SAMP_FREQ);

  /* Initialize First-Order High Pass Filter */
  HPF_t hpfHandler;
  HPF_init(&hpfHandler, HPF_CORNER_FREQ, SAMP_FREQ);

  /* Initialize Delay Effect Filter */
  DelayFilter_t dftHandler;
  Delay_Filter_init(&dftHandler, DELAY_SIZE, DELAY_CUTOFF);

  /* Initialize Noise Gate Filter */
  NoiseGateFilt_t ngfHandler;
  NoiseGate_init(&ngfHandler, NGF_THRESHOLD, NGF_ATTACKTIME, NGF_RELEASETIME, NGF_HOLDTIME, SAMP_FREQ);

  /* Initialize Delay Filter */
  Delay_t dfHandler;
  Delay_init(&dfHandler, DELAY_TIME_MS, DELAY_MIX, DELAY_FEEDBACK, SAMP_FREQ);
//
//  /* Initialize TIM2 to Begin Sample Collection */
//  TIM_init();

  /* Begin ADC Conversion to Continuously Collect Guitar Samples */
  ADC_collect();

  while (1)
  {
//	  /* Check if ARR has been Reached: Output and Collect New Sample */
//	  if (Output_Flag) {
//		  /* Output Value from Output Buffer to DAC (Converted to uint16_t from float) */
//		  DAC_Write((uint16_t) (FLOAT_TO_UINT16(outBuff[buffIDX]) * GAIN));
//
//		  /* Begin ADC Conversion to Collect Guitar Sample */
//		  ADC_collect();
//
//		  /* Clear ARR Flag */
//		  Output_Flag = CLEAR;
//	  }
//
//	  /* Process Collected Sample */
//	  if (Input_Flag) {
////		  /* Store Sample Value from ADC into Input Buffer (Converted to float from uint16_t) */
////		  inBuff[buffIDX] = (float) (UINT16_TO_FLOAT(GtrSamp_DigVal) * GAIN);
////
////		  /* Increment Input Buffer Counter; Apply FFT Once Full */
////		  buffIDX++;
////		  if (buffIDX == BUFFER_SIZE) {
////			  /* Reset Input Buffer Counter */
////			  buffIDX = 0x0;
////
////			  /* Apply FFT and Store to Output Buffer */
////			  arm_rfft_fast_f32(&fftHandler, inBuff, outBuff, FFT);
////
////			  /* Apply Effects */
////			  //
////
////			  /* Apply iFFT and Replace Output Buffer */
////			  arm_rfft_fast_f32(&fftHandler, outBuff, outBuff, IFFT);
////		  }
//
//		  if (buffIDX < BUFFER_SIZE) {
//			  	inVal = INT16_TO_FLOAT(*(inBuffPtr++));
//
//				/* Apply Signal Modification */
//				outVal = (inVal * GAIN);
//
//				/* Convert Output to int16_t and Send to DAC */
//				*(outBuffPtr++) = (int16_t) FLOAT_TO_INT16(outVal);
//				DAC1->DHR12R1 = (int16_t) FLOAT_TO_INT16(outVal);
//		  } else {
//			  buffIDX = 0;
//			  inBuffPtr = &inBuff[0];
//			  outBuffPtr = &outBuff[0];
//		  }
////		  DAC_Write((GtrSamp_DigVal * GAIN));
//		  /* Clear ADC Flag */
//		  Input_Flag = CLEAR;
//	  }


	  if (Data_Ready_Flag) {
		  /* Process Ready Data While DMA Transfer Continues */
		  processData(&lpfHandler, &hpfHandler, &dftHandler, &ngfHandler, &dfHandler);

		  /* Debug: Toggle LED */
		  LED_Debug_1_toggle();

		  /* Clear Flag */
		  Data_Ready_Flag = CLEAR;
	  }
  }
}


/* Functions */
/* Process Stored Data in Buffer */
void processData(LPF_t *lpf, HPF_t *hpf, DelayFilter_t *dft, NoiseGateFilt_t *ngf,
				Delay_t *df) {
	static float inVal, outVal;

	/* Process Half of the Buffer */
	for (uint16_t i = 0x0; i < (BUFFER_SIZE / 2); i++) {
		/* Take Input and Convert to Float */
		inVal = INT16_TO_FLOAT(*(inBuffPtr++));
		if (inVal > 1.0f) {
			inVal -= 2.0f;
		}

		/* Apply Signal Modification */
//		inVal = HPF_apply(hpf, inVal);
//		inVal = LPF_apply(lpf, inVal);
		inVal = Delay_update(df, inVal);
//		inVal = NoiseGate_update(ngf, inVal);
//		inVal = Delay_Filter_apply(dft, inVal);
		outVal = (inVal * GAIN);



		/* Convert Output to int16_t and Send to DAC */
		*(outBuffPtr++) = (int16_t) FLOAT_TO_INT16(outVal);
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
