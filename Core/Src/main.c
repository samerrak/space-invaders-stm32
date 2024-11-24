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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    float32_t q;  // Process noise covariance
    float32_t r;  // Measurement noise covariance
    float32_t x;  // Filtered value
    float32_t p;  // Estimate error covariance
    float32_t k;  // Kalman gain
} kalman_state;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ARM_MATH_CM4
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef huart1;

osThreadId readSensorHandle;
osThreadId buttonCheckHandle;
osThreadId printUARTHandle;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C2_Init(void);
static void MX_USART1_UART_Init(void);
void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);
void StartTask03(void const * argument);
void tilt_detection(void);
float kalman_filter_CMSIS(kalman_state *kstate, float measurement);

/* USER CODE BEGIN PFP */

void readSensors();
void printOne(char* string, float data);
void printThree(char* string, int data1, int data2, int data3);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Global variables for sensor measurements */
#define ACCEL_THRESHOLD 10.0f  // Threshold for tilt detection
#define X_MAP_SIZE 80

float pitch_angle = 0.0f;  // Pitch angle (forward/backward)
float roll_angle = 0.0f;   // Roll angle (left/right)
int16_t x_position = X_MAP_SIZE/2;

/* Kalman Filter States */
kalman_state kalman_x = {0.07f, 2.0f, 0.0f, 1.0f, 0.0f};
kalman_state kalman_y = {0.07f, 2.0f, 0.0f, 1.0f, 0.0f};
kalman_state kalman_z = {0.07f, 2.0f, 0.0f, 1.0f, 0.0f};

int16_t raw_acceleration[3];    // Raw accelerometer data
int16_t filtered_acceleration[3];  // Filtered accelerometer data (using Kalman filter)

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* MCU Configuration*/
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  BSP_ACCELERO_Init();  // Initialize the accelerometer

  while (1)
  {
	  readSensors();
	  tilt_detection();
	  HAL_Delay(50);
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
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
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

void printOne(char* string, float data) {
    char output[120];
    sprintf(output, string, (int)data);
    uint16_t len = strlen(output);
    HAL_UART_Transmit(&huart1, (uint8_t *)output, len, 120);
}

void printThree(char* string, int data1, int data2, int data3) {
    char output[60];
    sprintf(output, string, data1, data2, data3);
    uint16_t len = strlen(output);
    HAL_UART_Transmit(&huart1, (uint8_t *)output, len, 120);
}

float kalman_filter_CMSIS(kalman_state *kstate, float measurement) {
    float temp_p, temp_k, temp_x;

    // Predict
    arm_add_f32(&kstate->p, &kstate->q, &temp_p, 1);  // p = p + q

    // Update
    arm_add_f32(&temp_p, &kstate->r, &temp_k, 1);  // temp_k = p + r
    temp_k = temp_p / temp_k;  // k = p / (p + r)

    float temp_diff;
    arm_sub_f32(&measurement, &kstate->x, &temp_diff, 1);  // temp_diff = measurement - x
    arm_mult_f32(&temp_k, &temp_diff, &temp_x, 1);  // temp_x = k * (measurement - x)
    arm_add_f32(&kstate->x, &temp_x, &temp_x, 1);  // x = x + k * (measurement - x)

    // Update covariance
    float temp_p_update;
    float one_minus_k = 1.0f - temp_k;
    arm_mult_f32(&one_minus_k, &temp_p, &temp_p_update, 1);  // p = (1 - k) * p

    // Save updated values
    kstate->p = temp_p_update;
    kstate->x = temp_x;
    kstate->k = temp_k;

    return kstate->x;
}

/* Read sensor */
void readSensors(void) {
    // Read raw accelerometer data
    BSP_ACCELERO_AccGetXYZ(raw_acceleration);

    // Apply Kalman filter to each axis
    filtered_acceleration[0] = kalman_filter_CMSIS(&kalman_x, (float32_t)raw_acceleration[0]);
    filtered_acceleration[1] = kalman_filter_CMSIS(&kalman_y, (float32_t)raw_acceleration[1]);
    filtered_acceleration[2] = kalman_filter_CMSIS(&kalman_z, (float32_t)raw_acceleration[2]);
}

float calculate_pitch(int16_t *accel_data) {
    // Pitch = atan2(accel_data[0], sqrt(accel_data[1]^2 + accel_data[2]^2))
    return atan2(accel_data[0], sqrt(accel_data[1] * accel_data[1] + accel_data[2] * accel_data[2])) * 180.0 / M_PI;
}

float calculate_roll(int16_t *accel_data) {
    // Roll = atan2(accel_data[1], sqrt(accel_data[0]^2 + accel_data[2]^2))
    return atan2(accel_data[1], sqrt(accel_data[0] * accel_data[0] + accel_data[2] * accel_data[2])) * 180.0 / M_PI;
}

/* Tilt detection logic */
void tilt_detection() {
    float pitch, roll;

    // Calculate pitch (forward-backward tilt)
    pitch = calculate_pitch(filtered_acceleration);

    // Calculate roll (left-right tilt)
    roll = calculate_roll(filtered_acceleration);

    //printOne("Pitch: %.2f\n", pitch);  // Print pitch angle
    //printOne("Roll: %.2f\n", roll);    // Print roll angle

    // Detect tilt direction for pitch (forward-backward)
    if (pitch > ACCEL_THRESHOLD && x_position < 80) {
        printOne("Tilted right, position: %d\r\n", x_position);
        x_position++;
    } else if (pitch < -ACCEL_THRESHOLD && x_position > 0) {
        printOne("Tilted left, position: %d\r\n", x_position);
        x_position--;
    } else {
        printOne("No change in position: %d\r\n", x_position);
    }

    // Detect tilt direction for roll (left-right)
    if (roll > ACCEL_THRESHOLD) {
        printOne("Tilted forward\r\n", 0);  // Print tilt direction
    } else if (roll < -ACCEL_THRESHOLD) {
        printOne("Tilted backward\r\n", 0);  // Print tilt direction
    } else {
        //printOne("No significant roll tilt\n", 0);  // Print if no significant tilt
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
  * @brief  Function implementing the readSensor thread.
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
	osDelay(1000);
	readSensors();
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the buttonCheck thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(400);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
* @brief Function implementing the printUART thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask03 */
void StartTask03(void const * argument)
{
  /* USER CODE BEGIN StartTask03 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END StartTask03 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
