/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define ACC 0b0011001
#define WHO_AM_I_A 0x0F
#define CTRL_REG1_A 0x20
#define CTRL_REG2_A 0x21
#define CTRL_REG3_A 0x22
#define CTRL_REG4_A 0x23
#define CTRL_REG5_A 0x24
#define CTRL_REG6_A 0x25
#define STATUS_REG_A 0x27
#define OUT_X_L_A 0x28
#define OUT_X_H_A 0x29
#define OUT_Y_L_A 0x2A
#define OUT_Y_H_A 0x2B
#define OUT_Z_L_A 0x2C
#define OUT_Z_H_A 0x2D

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();
	MX_I2C1_Init();

	/* USER CODE BEGIN 2 */
	__HAL_RCC_I2C1_CLK_ENABLE();
	char aTxBuffer[16];
	char clear[7] = "\x1B[2J";

	/*
	* Clear console
	*/
	HAL_UART_Transmit(&huart2,  (uint8_t*)clear, sizeof(clear), 100);

	/*
	 * I2C SCANNER
	 */
	HAL_UART_Transmit(&huart2,  (uint8_t*)"Scanning\n\r", 11, 100);
	for (uint8_t i = 0; i < 128; i++) {
		if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i<<1), 3, 5) == HAL_OK) {
			HAL_UART_Transmit(&huart2, (uint8_t*)aTxBuffer, sprintf(aTxBuffer, "%d\n\r", i), 100);
		}
	}
	HAL_UART_Transmit(&huart2,  (uint8_t*)"Scanned\n\r", 10, 100);


	/*
	 * Accelerometer
	 */
	/*
	 * Check Communication from Accelerometer
	 */
	char acc_enabled[30] = "Accelerometer Enabled\n\r";
	uint8_t who_am_i_a_val;
	HAL_StatusTypeDef who_am_i_a_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1 , WHO_AM_I_A, 1, &who_am_i_a_val, 1, 50);
	char ACC_Buffer[32];
	if (who_am_i_a_status == HAL_OK && who_am_i_a_val == 51) {
		HAL_UART_Transmit(&huart2, (uint8_t*)acc_enabled , sizeof(acc_enabled), 100);
	} else if (who_am_i_a_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "%d\n\r", who_am_i_a_status), 100);
	}
	/*
	 * Set Control Registers
	 */
	uint8_t CTRL_REG1_A_val = 0x57;
	uint8_t CTRL_REG2_A_val = 0x00;
	uint8_t CTRL_REG3_A_val = 0x00;
	uint8_t CTRL_REG4_A_val = 0x81;

	HAL_StatusTypeDef CTRL_REG1_A_status = HAL_I2C_Mem_Write(&hi2c1, (ACC<<1), CTRL_REG1_A, 1, &CTRL_REG1_A_val, 1, 50);
	HAL_StatusTypeDef CTRL_REG2_A_status = HAL_I2C_Mem_Write(&hi2c1, (ACC<<1), CTRL_REG2_A, 1, &CTRL_REG2_A_val, 1, 50);
	HAL_StatusTypeDef CTRL_REG3_A_status = HAL_I2C_Mem_Write(&hi2c1, (ACC<<1), CTRL_REG3_A, 1, &CTRL_REG3_A_val, 1, 50);
	HAL_StatusTypeDef CTRL_REG4_A_status = HAL_I2C_Mem_Write(&hi2c1, (ACC<<1), CTRL_REG4_A, 1, &CTRL_REG4_A_val, 1, 50);

	if (CTRL_REG1_A_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Failed REG1:  %d\n\r", CTRL_REG1_A_status), 100);
	}

	if (CTRL_REG2_A_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Failed REG2:  %d\n\r", CTRL_REG2_A_status), 100);
	}

	if (CTRL_REG3_A_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Failed REG3:  %d\n\r", CTRL_REG3_A_status), 100);
	}

	if (CTRL_REG4_A_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Failed REG4:  %d\n\r", CTRL_REG4_A_status), 100);
	}

	/*
	 * Start up
	 */
	HAL_Delay(500);

	/*
	 * Check Status
	 */
	uint8_t STATUS_REG_A_val = 0;

	HAL_StatusTypeDef STATUS_REG_A_status;

	/*
	 * Read First
	 */
	uint8_t OUT_X_L_A_val = 0x00;
	uint8_t OUT_X_H_A_val = 0x00;
	int16_t OUT_X_A_val = 0x00;

	uint8_t OUT_Y_L_A_val = 0x00;
	uint8_t OUT_Y_H_A_val = 0x00;
	int16_t OUT_Y_A_val = 0x00;

	uint8_t OUT_Z_L_A_val = 0x00;
	uint8_t OUT_Z_H_A_val = 0x00;
	int16_t OUT_Z_A_val = 0x00;

	HAL_StatusTypeDef OUT_X_L_A_status = 0x00;
	HAL_StatusTypeDef OUT_X_H_A_status = 0x00;

	HAL_StatusTypeDef OUT_Y_L_A_status = 0x00;
	HAL_StatusTypeDef OUT_Y_H_A_status = 0x00;

	HAL_StatusTypeDef OUT_Z_L_A_status = 0x00;
	HAL_StatusTypeDef OUT_Z_H_A_status = 0x00;

	while (1)
	{


		STATUS_REG_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, STATUS_REG_A, 1, &STATUS_REG_A_val, 1, 50);
//		HAL_UART_Transmit(&huart2,  (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Status Register: %d\n\r", STATUS_REG_A_val), 100);
		if (STATUS_REG_A_status == HAL_OK && ((STATUS_REG_A_val & 0x08)>>3) == 1) {
			/*
			 * Sampling
			 */

			uint8_t sample = 5;
			int16_t arr_x[sample];
			int16_t arr_y[sample];
			int16_t arr_z[sample];
			for (int i=0;i<sample;i++) {
				OUT_X_L_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, OUT_X_L_A, 1, &OUT_X_L_A_val, 1, 50);
				OUT_X_H_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, OUT_X_H_A, 1, &OUT_X_H_A_val, 1, 50);

				OUT_Y_L_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, OUT_Y_L_A, 1, &OUT_Y_L_A_val, 1, 50);
				OUT_Y_H_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, OUT_Y_H_A, 1, &OUT_Y_H_A_val, 1, 50);

				OUT_Z_L_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, OUT_Z_L_A, 1, &OUT_Z_L_A_val, 1, 50);
				OUT_Z_H_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, OUT_Z_H_A, 1, &OUT_Z_H_A_val, 1, 50);

				if (OUT_X_L_A_status == HAL_OK && OUT_X_H_A_status == HAL_OK) {
//					OUT_X_A_val = (((OUT_X_H_A_val<<8) | OUT_X_L_A_val)>>6);
					OUT_X_A_val = OUT_X_H_A_val;
					OUT_X_A_val <<= 8;
					OUT_X_A_val |= OUT_X_L_A_val;
					OUT_X_A_val >>= 6;
			//		HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Low: %x\n\r", OUT_Y_L_A_val), 100);
			//		HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "High: %x\n\r", OUT_Y_H_A_val), 100);
//					HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "X: %05d ", OUT_X_A_val), 100);
				}

				if (OUT_Y_L_A_status == HAL_OK && OUT_Y_H_A_status == HAL_OK) {
//					OUT_Y_A_val = (((OUT_Y_H_A_val<<8) | OUT_Y_L_A_val)>>6);
					OUT_Y_A_val = OUT_Y_H_A_val;
					OUT_Y_A_val <<= 8;
					OUT_Y_A_val |= OUT_Y_L_A_val;
					OUT_Y_A_val >>= 6;
			//		HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Low: %x\n\r", OUT_Y_L_A_val), 100);
			//		HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "High: %x\n\r", OUT_Y_H_A_val), 100);
//					HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Y: %05d ", OUT_Y_A_val), 100);
				}

				if (OUT_Z_L_A_status == HAL_OK && OUT_Z_H_A_status == HAL_OK) {
//					OUT_Z_A_val = (((OUT_Z_H_A_val<<8) | OUT_Z_L_A_val)>>6);
					OUT_Z_A_val = OUT_Z_H_A_val;
					OUT_Z_A_val <<= 8;
					OUT_Z_A_val |= OUT_Z_L_A_val;
					OUT_Z_A_val >>= 6;
			//		HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Low: %x\n\r", OUT_Y_L_A_val), 100);
			//		HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "High: %x\n\r", OUT_Y_H_A_val), 100);
//					HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Z: %05d\n\r", OUT_Z_A_val), 100);
				}
//				HAL_Delay(500);
				arr_x[i] = OUT_X_A_val;
				arr_y[i] = OUT_Y_A_val;
				arr_z[i] = OUT_Z_A_val;
			}

			/*
			 * Average
			 */
			float avg_x = 0;
			float avg_y = 0;
			float avg_z = 0;
			for (int i=0;i<sample;i++) {
				avg_x += arr_x[i];
				avg_y += arr_y[i];
				avg_z += arr_z[i];
			}
			avg_x = (avg_x / sample) * (4.0 / 1023);
			avg_y = (avg_y / sample) * (4.0 / 1023);
			avg_z = (avg_z / sample) * (4.0 / 1023);

			HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "X: % 06.5fG  ", avg_x), 100); // @suppress("Float formatting support")
			HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Y: % 06.5fG  ", avg_y), 100); // @suppress("Float formatting support")
			HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Z: % 06.5fG  \n\r", avg_z), 100); // @suppress("Float formatting support")



		} else {
			HAL_UART_Transmit(&huart2,  (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Not Good \n\r"), 100);
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 9600;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
