/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "arm_math.h"
#include <stdio.h>
#include "util.h"
#include "kalman.h"
#include "ui.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ARM_MATH_CM4

// sound
#define SIZE1K ((float) 44.0)
#define SIZE15K ((float) 30.0)
#define SIZE2K ((float) 22.0)

#define VREFINT_CAL_ADDRESS ((uint16_t*) (0x1FFF75AA))
#define VREFINT_CAL (*VREFINT_CAL_ADDRESS)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac1_ch1;

I2C_HandleTypeDef hi2c2;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;

osThreadId defaultTaskHandle;
osThreadId processingDataHandle;
osThreadId soundTaskHandle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_DAC1_Init(void);
static void MX_TIM2_Init(void);
void StartDefaultTask(void const * argument);
void StartProcessData(void const * argument);
void StartSoundTask(void const * argument);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Constants */
#define ACCEL_THRESHOLD 10.0f  // Threshold for tilt detection

const char* map_names[NUM_MAPS] = {
    "Zelko's Dungeon",
    "Zilander",
    "Zoolander",
	"Zoolander 2",
	"Zoo york"
};



/* Tilting detection */
int16_t x_position = X_MAP_SIZE/2 ;

/* Kalman Filter States */
kalman_state kalman_x = {0.07f, 2.0f, 0.0f, 1.0f, 0.0f};
kalman_state kalman_y = {0.07f, 2.0f, 0.0f, 1.0f, 0.0f};
kalman_state kalman_z = {0.07f, 2.0f, 0.0f, 1.0f, 0.0f};

/* Acceleration data */
int16_t raw_acceleration[3];    // Raw accelerometer data
int16_t filtered_acceleration[3];  // Filtered accelerometer data (using Kalman filter)


char display[25][60] = {0};
char ui_string[1760] = {0};

struct Position alien_positions[70] = {0};
int aliens_remaining = 0;
struct Position bullet_positions[300] = {0};

// Bullet
int fireBullet = 0;
// Timestamp
int timestamp = 0;
int mode;

// Gameplay
uint8_t gameOver = 0;

// Main menu
int buttonPressed = 0;
int main_menu = 1;
int printed_menu = 0;
float init_p = 0.0;
float pressure = 0.0;

// sound
int soundFlag = 0;

float vref;

float sin1kHz_f[44] = {2048.0, 2242.3071925224585, 2432.658850940832, 2615.1799644185753, 2786.154929432709, 2942.1031887466293, 3079.850085518347, 3196.591490158839, 3289.9508843240355, 3358.027739974994, 3399.4362086481, 3413.333333333333, 3399.4362086481, 3358.027739974994, 3289.9508843240355, 3196.591490158839, 3079.850085518347, 2942.1031887466293, 2786.154929432709, 2615.1799644185753, 2432.658850940832, 2242.307192522459, 2048.0000000000005, 1853.6928074775415, 1663.3411490591684, 1480.8200355814242, 1309.8450705672908, 1153.8968112533707, 1016.1499144816528, 899.4085098411604, 806.0491156759641, 737.9722600250062, 696.5637913518999, 682.6666666666666, 696.5637913518997, 737.9722600250061, 806.049115675964, 899.4085098411603, 1016.1499144816528, 1153.896811253371, 1309.8450705672908, 1480.8200355814242, 1663.341149059168, 1853.692807477541};
float sin15kHz_f[30]= {2048.0, 2331.8687618631807, 2603.3310966794925, 2850.5227977966565, 3062.6404017184686, 3230.4133513003535, 3346.50916358165, 3405.853894476149, 3405.8538944761494, 3346.50916358165, 3230.4133513003535, 3062.640401718469, 2850.522797796657, 2603.331096679493, 2331.868761863181, 2048.0000000000005, 1764.1312381368198, 1492.668903320508, 1245.4772022033435, 1033.3595982815314, 865.5866486996466, 749.4908364183503, 690.1461055238508, 690.1461055238507, 749.4908364183502, 865.5866486996458, 1033.3595982815305, 1245.477202203343, 1492.6689033205064, 1764.1312381368184};
float sin2kHz_f[22] = {2048.0, 2432.658850940832, 2786.154929432709, 3079.850085518347, 3289.9508843240355, 3399.4362086481, 3399.4362086481, 3289.9508843240355, 3079.850085518347, 2786.154929432709, 2432.658850940832, 2048.0000000000005, 1663.3411490591684, 1309.8450705672908, 1016.1499144816528, 806.0491156759641, 696.5637913518999, 696.5637913518997, 806.049115675964, 1016.1499144816528, 1309.8450705672908, 1663.341149059168};

uint16_t sin1kHz[44];
uint16_t sin15kHz[30];
uint16_t sin2kHz[22];

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_DAC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  BSP_ACCELERO_Init();  // Initialize the accelerometer
  BSP_PSENSOR_Init();

  if (HAL_ADC_Start(&hadc1))
  {
      Error_Handler();
  }
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
  HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);   //wait for completion
  float raw_voltage = HAL_ADC_GetValue(&hadc1);		  //read sensor's digital value
  HAL_ADC_Stop(&hadc1);
  vref = 3.0f * VREFINT_CAL/raw_voltage;

  for (int y = 0; y<SIZE1K ; y++){
	  sin1kHz[y] = (int) (sin1kHz_f[y]/vref*2);
  }

  for (int y = 0; y<SIZE15K ; y++){
  	  sin15kHz[y] =(int) (sin15kHz_f[y]/vref*2); // +1 for positive and multiplication for amplitude
  }

  for (int y = 0; y<SIZE2K ; y++){
  	  sin2kHz[y] = (int) (sin2kHz_f[y]/vref*2); // +1 for positive and multiplication for amplitude
  }

  if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
  {
    Error_Handler();
  }


  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityLow, 0, 256);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of processingData */
  osThreadDef(processingData, StartProcessData, osPriorityNormal, 0, 256);
  processingDataHandle = osThreadCreate(osThread(processingData), NULL);

  /* definition and creation of soundTask */
  osThreadDef(soundTask, StartSoundTask, osPriorityIdle, 0, 128);
  soundTaskHandle = osThreadCreate(osThread(soundTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 60;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_T2_TRGO;
  sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_ABOVE_80MHZ;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_DISABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT2 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x30A175AB;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 2727;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 1843200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV4;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMAMUX1_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(myLed1_GPIO_Port, myLed1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : myButton_Pin */
  GPIO_InitStruct.Pin = myButton_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(myButton_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : myLed1_Pin */
  GPIO_InitStruct.Pin = myLed1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(myLed1_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback (uint16_t GPIO_Pin) {
	if (GPIO_Pin == myButton_Pin) {
		soundFlag = 1;
		if (main_menu == 1){
			buttonPressed++;
			printed_menu=0;
		} else {
			shoot();
		}
	}
}



void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // Report stack overflow
    printf("Stack overflow detected in task: %s\n", pcTaskName);

    // Optionally halt the system for debugging
    while (1);
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
	/* Infinite loop */
	for(;;)
	{
		osDelay(100);

		if (main_menu == 1)
		{

			if (printed_menu == 0) {
				printed_menu = 1;
				print_main_menu();
				init_p = BSP_PSENSOR_ReadPressure() + 5;

			}
			pressure = BSP_PSENSOR_ReadPressure();
			if (pressure > init_p) {
				char* buffer = (char*)malloc(128 * sizeof(char));
					if (buffer == NULL) {
						// Handle allocation failure
						return;
					}
				// Clear buffer
				memset(buffer, 0, 512);
				// Add header
				strcpy(buffer, "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nYou have selected the map:");

				// Add map names
				strcat(buffer, map_names[buttonPressed%NUM_MAPS]);
				// Transmit
				HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), 500);
				free(buffer);
				main_menu = 0;
				HAL_UART_Transmit(&huart1, (uint8_t*)"\033[2J", strlen("\033[2J"), 500);
				aliens_remaining = ((buttonPressed)%NUM_MAPS+1)*10;
				for (int i = 0; i < 300; i++) {
					bullet_positions[i].row = -1;
					bullet_positions[i].col = -1;
				}
			}
		}

		if (main_menu != 1) {
			uint8_t moveBullets = 0;
			uint8_t moveAliens = 0;

			if (timestamp % 2 == 0) {
				moveBullets = 1;
			}

			if (timestamp % 12 == 0) {
				if (timestamp < ((buttonPressed)%NUM_MAPS+1)*24 && timestamp % 24 == 0) start_wave((timestamp)/24);
				moveAliens = 1;
			}
			HAL_UART_Transmit(&huart1, (uint8_t*)"\033[H", strlen("\033[H"), 100);
			// HAL_UART_Transmit(&huart1, (uint8_t*)"\033[2K", strlen("\033[2K"), 100);
			if (!gameOver) {
				gameOver = compute_new_UI_frame(moveBullets, moveAliens);
				timestamp++;
			} else {

				if(aliens_remaining == 0){
					print_win();
				} else {
					print_game_over();
				}
			}
	}

	}
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartProcessData */
/**
* @brief Function implementing the processingData thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartProcessData */
void StartProcessData(void const * argument)
{
  /* USER CODE BEGIN StartProcessData */
  /* Infinite loop */
  for(;;)
  {
    osDelay(150);
    if (main_menu != 1)
	{
    	BSP_ACCELERO_AccGetXYZ(raw_acceleration);
		 // Apply Kalman filter to each axis
		filtered_acceleration[0] = kalman_filter_CMSIS(&kalman_x, (float32_t)raw_acceleration[0]);
		filtered_acceleration[1] = kalman_filter_CMSIS(&kalman_y, (float32_t)raw_acceleration[1]);
		filtered_acceleration[2] = kalman_filter_CMSIS(&kalman_z, (float32_t)raw_acceleration[2]);
		x_position = tilt_detection(filtered_acceleration, x_position);
	}



  }
  /* USER CODE END StartProcessData */
}

/* USER CODE BEGIN Header_StartSoundTask */
/**
* @brief Function implementing the soundTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartSoundTask */
void StartSoundTask(void const * argument)
{
  /* USER CODE BEGIN StartSoundTask */
  /* Infinite loop */
  for(;;)
  {
	osDelay(10);
	if (soundFlag == 1)
	{
		soundFlag = 0;
		HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint16_t *)sin2kHz, SIZE2K, DAC_ALIGN_12B_R);
		osDelay(100); // HAL_Delay wrecks the program, callbacks must be short
		HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
	}
  }
  /* USER CODE END StartSoundTask */
}

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
