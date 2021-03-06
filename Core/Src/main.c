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
#include <math.h>

/* Private typedef -----------------------------------------------------------*/
#define PI 3.141592654
#define MODE 1 //MODE 0 -> CONSOLE. MODE 1 -> MATLAB

/*
 * Registers
 */
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

#define MAG 0b0011110
#define WHO_AM_I_M 0x4F
#define CFG_REG_A_M 0x60
#define CFG_REG_B_M 0x61
#define CFG_REG_C_M 0x62
#define STATUS_REG_M 0x67
#define OUTX_L_REG_M 0x68
#define OUTX_H_REG_M 0x69
#define OUTY_L_REG_M 0x6A
#define OUTY_H_REG_M 0x6B
#define OUTZ_L_REG_M 0x6C
#define OUTZ_H_REG_M 0x6D

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
void KALMAN(float U, float *P, float *U_hat, float *K);

/*
 * Kalman Filter Variables
 */
static const float R = 0.3; //Observation noise
static const float H = 1.0; //Observation model
static float Q = 3; //Process noise

//Magnetomer X Axis
//Prediction
float P_x_m = 0;
//Updated
float U_hat_x_m = 0;
//Kalman Constant
float K_x_m = 0;

//Magnetomer Y Axis
//Prediction
float P_y_m = 0;
//Updated
float U_hat_y_m = 0;
//Kalman Constant
float K_y_m = 0;

//Magnetomer Z Axis
//Prediction
float P_z_m = 0;
//Updated
float U_hat_z_m = 0;
//Kalman Constant
float K_z_m = 0;

/*
 * Step length
 */
float step_length = 0.7;//in meter

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
	char aTxBuffer[16];//Buffer for UART string
	char clear[7] = "\x1B[2J";//Clear Console Command (macOS Terminal)

	/*
	 * Start LED ON
	 */
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 1);//Signals the system is on

	/*
	* Clear console
	*/
	HAL_UART_Transmit(&huart2,  (uint8_t*)clear, sizeof(clear), 100);

	/*
	 * I2C SCANNER
	 */
	HAL_UART_Transmit(&huart2,  (uint8_t*)"I2C Scan\n\r", 11, 100);
	for (uint8_t i = 0; i < 128; i++) {
		if (HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t)(i<<1), 3, 5) == HAL_OK) {
			HAL_UART_Transmit(&huart2, (uint8_t*)aTxBuffer, sprintf(aTxBuffer, "%d\n\r", i), 100);
		}
	}
	HAL_UART_Transmit(&huart2,  (uint8_t*)"Scanned\n\r", 10, 100);

	/*
	 * Start up
	 */
	HAL_Delay(500);

	/*
	 * *************************************** Accelerometer Start Up Sequence ***************************************
	 */
	/*
	 * Check Communication from Accelerometer
	 */
	char acc_enabled[30] = "Accelerometer Enabled\n\r";// UART Buffer
	uint8_t who_am_i_a_val;
	HAL_StatusTypeDef who_am_i_a_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1 , WHO_AM_I_A, 1, &who_am_i_a_val, 1, 50);// Read WHOAMI
	char ACC_Buffer[32];
	if (who_am_i_a_status == HAL_OK && who_am_i_a_val == 51) {// Who Am I register check
		HAL_UART_Transmit(&huart2, (uint8_t*)acc_enabled , sizeof(acc_enabled), 100);//OK
	} else if (who_am_i_a_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "%d\n\r", who_am_i_a_status), 100);//BAD
	}
	/*
	 * Set Control Registers
	 */
	uint8_t CTRL_REG1_A_val = 0x57;
	uint8_t CTRL_REG2_A_val = 0x00;
	uint8_t CTRL_REG3_A_val = 0x00;
	uint8_t CTRL_REG4_A_val = 0x81;

	/*
	 * Set
	 */
	HAL_StatusTypeDef CTRL_REG1_A_status = HAL_I2C_Mem_Write(&hi2c1, (ACC<<1), CTRL_REG1_A, 1, &CTRL_REG1_A_val, 1, 50);
	HAL_StatusTypeDef CTRL_REG2_A_status = HAL_I2C_Mem_Write(&hi2c1, (ACC<<1), CTRL_REG2_A, 1, &CTRL_REG2_A_val, 1, 50);
	HAL_StatusTypeDef CTRL_REG3_A_status = HAL_I2C_Mem_Write(&hi2c1, (ACC<<1), CTRL_REG3_A, 1, &CTRL_REG3_A_val, 1, 50);
	HAL_StatusTypeDef CTRL_REG4_A_status = HAL_I2C_Mem_Write(&hi2c1, (ACC<<1), CTRL_REG4_A, 1, &CTRL_REG4_A_val, 1, 50);

	/*
	 * Check if successful
	 */
	if (CTRL_REG1_A_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Failed REG1:  %d\n\r", CTRL_REG1_A_status), 100);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);//Turn off LED
	}

	if (CTRL_REG2_A_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Failed REG2:  %d\n\r", CTRL_REG2_A_status), 100);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);//Turn off LED
	}

	if (CTRL_REG3_A_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Failed REG3:  %d\n\r", CTRL_REG3_A_status), 100);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);//Turn off LED
	}

	if (CTRL_REG4_A_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Failed REG4:  %d\n\r", CTRL_REG4_A_status), 100);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);//Turn off LED
	}

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


	/*
	 * *************************************** Magnetometer Start Up Sequence ***************************************
	 */
	/*
	 * Check Communication from Magnetometer
	 */
	char mag_enabled[50] = "Magnetometer Enabled\n\r";//UART Buffer
	uint8_t who_am_i_m_val;
	HAL_StatusTypeDef who_am_i_m_status = HAL_I2C_Mem_Read(&hi2c1, (MAG<<1)|0x1 , WHO_AM_I_M, 1, &who_am_i_m_val, 1, 50);// Read WHOAMI
	char MAG_Buffer[32];
	if (who_am_i_m_status == HAL_OK && who_am_i_m_val == 64) {//Check WHO AM I register
		HAL_UART_Transmit(&huart2, (uint8_t*)mag_enabled , sizeof(mag_enabled), 100);//OK
	} else if (who_am_i_m_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)MAG_Buffer, sprintf(MAG_Buffer, "%d\n\r", who_am_i_m_status), 100);//BAD
	}
	/*
	 * Set Control Registers
	 */
	uint8_t CFG_REG_A_M_val = 0x8C;
	uint8_t CFG_REG_B_M_val = 0x03;
	uint8_t CFG_REG_C_M_val = 0x10;

	/*
	 * Set
	 */
	HAL_StatusTypeDef CFG_REG_A_M_Status = HAL_I2C_Mem_Write(&hi2c1, (MAG<<1), CFG_REG_A_M, 1, &CFG_REG_A_M_val, 1, 50);
	HAL_StatusTypeDef CFG_REG_B_M_status = HAL_I2C_Mem_Write(&hi2c1, (MAG<<1), CFG_REG_B_M, 1, &CFG_REG_B_M_val, 1, 50);
	HAL_StatusTypeDef CFG_REG_C_M_status = HAL_I2C_Mem_Write(&hi2c1, (MAG<<1), CFG_REG_C_M, 1, &CFG_REG_C_M_val, 1, 50);

	/*
	 * Check if successful
	 */
	if (CFG_REG_A_M_Status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)MAG_Buffer, sprintf(MAG_Buffer, "Failed REG1:  %d\n\r", CFG_REG_A_M_Status), 100);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);//Turn off LED
	}

	if (CFG_REG_B_M_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)MAG_Buffer, sprintf(MAG_Buffer, "Failed REG2:  %d\n\r", CFG_REG_B_M_status), 100);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);//Turn off LED
	}

	if (CFG_REG_C_M_status != HAL_OK) {
		HAL_UART_Transmit(&huart2,  (uint8_t*)MAG_Buffer, sprintf(MAG_Buffer, "Failed REG3:  %d\n\r", CFG_REG_C_M_status), 100);
		HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, 0);//Turn off LED
	}

	/*
	 * Check Status
	 */
	uint8_t STATUS_REG_M_val = 0;
	HAL_StatusTypeDef STATUS_REG_M_status;

	/*
	 * Read First
	 */
	uint8_t OUTX_L_REG_M_val = 0x00;
	uint8_t OUTX_H_REG_M_val = 0x00;
	int16_t OUTX_M_val = 0x00;

	uint8_t OUTY_L_REG_M_val = 0x00;
	uint8_t OUTY_H_REG_M_val = 0x00;
	int16_t OUTY_M_val = 0x00;

	uint8_t OUTZ_L_REG_M_val = 0x00;
	uint8_t OUTZ_H_REG_M_val = 0x00;
	int16_t OUTZ_M_val = 0x00;

	HAL_StatusTypeDef OUTX_L_M_status = 0x00;
	HAL_StatusTypeDef OUTX_H_M_status = 0x00;

	HAL_StatusTypeDef OUTY_L_M_status = 0x00;
	HAL_StatusTypeDef OUTY_H_M_status = 0x00;

	HAL_StatusTypeDef OUTZ_L_M_status = 0x00;
	HAL_StatusTypeDef OUTZ_H_M_status = 0x00;

	/*
	* Button De-bounce
	*/
	int pushed = 0;

	/*
	 * Tick
	 */
	uint32_t current_tick =  HAL_GetTick();

	/*
	 * Step Detector Variables
	 */
	uint8_t step_counting = 0;
	uint16_t steps = 0;
	uint32_t increase_prev = 0;

	/*
	 * Orientation Variables
	 */
	float initial_yaw = 0;
	uint8_t offset_measure = 0;
	float offset = 11.9758333333;
	float yaw = 0;

	/*
	 * Previous Coordinate
	 */
	float x_pos_prev = 0;
	float y_pos_prev = 0;

	/*
	 * Send Coordinate Flag
	 */
	uint8_t ready_to_send = 0;

	/*
	 * Stationary G values (offset)
	 */
	float stationary_x = 0;
	float stationary_y = 0;
	float stationary_z = 0;

	/*
	 * Average Accelerometer Variables
	 */
	float avg_x_a = 0;
	float avg_y_a = 0;
	float avg_z_a = 0;
	uint8_t offset_avg_count = 0;

	/*
	 * Average Magnetometer Variables
	 */
	float avg_x_m = 0;
	float avg_y_m = 0;
	float avg_z_m = 0;

	/*
	 * Moving Average - Angle Variables
	 */
	float angles[5];
	float angle_avg;
	for (int i = 0; i<sizeof(angles)/sizeof(float); i++) {
		angles[i] = 0;//Initialise Array
	}
	float turn_prev = 0;
	uint8_t angle_count = 0;

	//Loop
	while (1)
	{
		//Read if new data is available
		STATUS_REG_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, STATUS_REG_A, 1, &STATUS_REG_A_val, 1, 50);

		if (STATUS_REG_A_status == HAL_OK && ((STATUS_REG_A_val & 0x08)>>3) == 1) {//Yes
			/*
			 * Sampling
			 */
			uint8_t sample_a = 5;
			int16_t arr_x_a[sample_a];
			int16_t arr_y_a[sample_a];
			int16_t arr_z_a[sample_a];
			// Samples for five times
			for (int i=0;i<sample_a;i++) {
				STATUS_REG_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, STATUS_REG_A, 1, &STATUS_REG_A_val, 1, 50);
				while (((STATUS_REG_A_val & 0x08)>>3) != 1) {// Wait until new values are ready
					STATUS_REG_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, STATUS_REG_A, 1, &STATUS_REG_A_val, 1, 50);
				}

				// Store
				OUT_X_L_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, OUT_X_L_A, 1, &OUT_X_L_A_val, 1, 50);
				OUT_X_H_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, OUT_X_H_A, 1, &OUT_X_H_A_val, 1, 50);

				OUT_Y_L_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, OUT_Y_L_A, 1, &OUT_Y_L_A_val, 1, 50);
				OUT_Y_H_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, OUT_Y_H_A, 1, &OUT_Y_H_A_val, 1, 50);

				OUT_Z_L_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, OUT_Z_L_A, 1, &OUT_Z_L_A_val, 1, 50);
				OUT_Z_H_A_status = HAL_I2C_Mem_Read(&hi2c1, (ACC<<1)|0x1, OUT_Z_H_A, 1, &OUT_Z_H_A_val, 1, 50);

				// Bitwise operations X, Y, Z registers
				if (OUT_X_L_A_status == HAL_OK && OUT_X_H_A_status == HAL_OK) {
					OUT_X_A_val = OUT_X_H_A_val;
					OUT_X_A_val <<= 8;
					OUT_X_A_val |= OUT_X_L_A_val;
					OUT_X_A_val >>= 6;
				}

				if (OUT_Y_L_A_status == HAL_OK && OUT_Y_H_A_status == HAL_OK) {
					OUT_Y_A_val = OUT_Y_H_A_val;
					OUT_Y_A_val <<= 8;
					OUT_Y_A_val |= OUT_Y_L_A_val;
					OUT_Y_A_val >>= 6;
				}

				if (OUT_Z_L_A_status == HAL_OK && OUT_Z_H_A_status == HAL_OK) {
					OUT_Z_A_val = OUT_Z_H_A_val;
					OUT_Z_A_val <<= 8;
					OUT_Z_A_val |= OUT_Z_L_A_val;
					OUT_Z_A_val >>= 6;
				}
				arr_x_a[i] = (OUT_X_A_val);
				arr_y_a[i] = (OUT_Y_A_val);
				arr_z_a[i] = (OUT_Z_A_val);


			}

			//Averaging five samples
			for (int i=0;i<sample_a;i++) {
				avg_x_a += arr_x_a[i];
				avg_y_a += arr_y_a[i];
				avg_z_a += arr_z_a[i];
			}

			/*
			 * Average Calculation
			 */
			avg_x_a = (avg_x_a / sample_a) * (4.0 / 1023);
			avg_y_a = (avg_y_a / sample_a) * (4.0 / 1023);
			avg_z_a = (avg_z_a / sample_a) * (4.0 / 1023);

			/*
			 * Serial Transmit (debug)
			 */
			/*
			HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "% 06.5f,", avg_x_a), 100); // @suppress("Float formatting support")
			HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "% 06.5f,", avg_y_a), 100); // @suppress("Float formatting support")
			HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "% 06.5f\n", avg_z_a), 100); // @suppress("Float formatting support")
			*/

			current_tick = HAL_GetTick();//Current tick

			/*
			 * Peak Detector
			 */
			if (avg_x_a > -0.70 && step_counting == 0 && (current_tick - increase_prev) > 400 && ready_to_send == 0) {//Wait for 400ms till next step
				step_counting = 1;
				increase_prev = HAL_GetTick();
			}
			if (step_counting == 1 && avg_x_a < -0.70) {
				step_counting = 0;
				ready_to_send = 1;
				steps++;
			}
			if (avg_x_a > (stationary_x -0.028) && avg_x_a < (stationary_x + 0.028) && ready_to_send == 1 && step_counting == 0) {//Detect end of a step
				ready_to_send = 0;
				if (offset_measure == 0) {//If not in initialising stage
					float turn = initial_yaw - yaw - offset;
					/*
					 * ***** MOVING AVERAGE CALCULATION *****
					 */
					float temp_angle = fabs(turn - turn_prev);
					if (temp_angle > 30) {
						for (int i = 0; i<sizeof(angles)/sizeof(float); i++) {
							angles[i] = 0;// Re-initialise
						}
						angle_count = 0;
					}
					if (angle_count < 5) {
						angles[angle_count] = turn;
						angle_count++;
					}
					if (angle_count == 5) {
					    for(int i=5-1;i>0;i--)
					    {
					    	angles[i]=angles[i-1];
					    }
					    angles[0] = turn;
					}
					angle_avg = 0;
					for (int k=0; k<angle_count; k++) {
						angle_avg += angles[k];
					}
					angle_avg = angle_avg / (angle_count);

					/*
					 * Calculate X and Y coordinate
					 */
					float x_pos = x_pos_prev + step_length * 1 * cos(turn * PI/180);
					float y_pos = y_pos_prev + step_length * 1 * sin(turn * PI/180);

					/*
					 * Transmit
					 */
					if (MODE == 0) {//For Console
						HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Step : %03d, ", steps), 100);
						HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Heading : % 06.3f degrees,", angle_avg), 100);
						HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "X : % 06.3f meters,", y_pos), 100);
						HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Y : % 06.3f meters\n\r", x_pos), 100);
					}
					if (MODE == 1) {//For MATLAB
						HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "%f,", x_pos), 100);
						HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "%f\n\r", y_pos), 100);
					}

					//Store previous positions
					x_pos_prev = x_pos;
					y_pos_prev = y_pos;
					turn_prev = turn;

				}else if (offset_measure == 1 && (steps) <= 4) {//In initialising stage
					//Takes 5 samples initially and averages offset
					float turn = initial_yaw - yaw;
					HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Sample %d   Measuring %f\n\r", steps, turn), 100);
					offset += turn;
					offset_avg_count++;
				} else if (offset_measure == 1 && (steps) >= 5) {//End of initialising stage
					//Finished taking 5 samples
					offset = offset/offset_avg_count;
					HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Offset calculated %f\n\r", offset), 100);
					//Initialise and ready for measurement
					offset_measure = 0;
					steps = 0;
					offset_avg_count = 0;
					HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Ready\n\r"), 100);
				}
			}
		}


		/*
		 * Magnetometer
		 */

		STATUS_REG_M_status = HAL_I2C_Mem_Read(&hi2c1, (MAG<<1)|0x1, STATUS_REG_M, 1, &STATUS_REG_M_val, 1, 50);//Read if new data is available

		if (STATUS_REG_M_status == HAL_OK && ((STATUS_REG_M_val & 0x08)>>3) == 1) {
			/*
			 * Sampling
			 */
			uint8_t sample_m = 10;
			int16_t arr_x_m[sample_m];
			int16_t arr_y_m[sample_m];
			int16_t arr_z_m[sample_m];
			// Samples for ten times
			for (int i=0;i<sample_m;i++) {

				STATUS_REG_M_status = HAL_I2C_Mem_Read(&hi2c1, (MAG<<1)|0x1, STATUS_REG_M, 1, &STATUS_REG_M_val, 1, 50);
				while (((STATUS_REG_M_val & 0x08)>>3) != 1) {// Wait until new values are ready
					STATUS_REG_M_status = HAL_I2C_Mem_Read(&hi2c1, (MAG<<1)|0x1, STATUS_REG_M, 1, &STATUS_REG_M_val, 1, 50);
				}

				//Store
				OUTX_L_M_status = HAL_I2C_Mem_Read(&hi2c1, (MAG<<1)|0x1, OUTX_L_REG_M, 1, &OUTX_L_REG_M_val, 1, 50);
				OUTX_H_M_status = HAL_I2C_Mem_Read(&hi2c1, (MAG<<1)|0x1, OUTX_H_REG_M, 1, &OUTX_H_REG_M_val, 1, 50);

				OUTY_L_M_status = HAL_I2C_Mem_Read(&hi2c1, (MAG<<1)|0x1, OUTY_L_REG_M, 1, &OUTY_L_REG_M_val, 1, 50);
				OUTY_H_M_status = HAL_I2C_Mem_Read(&hi2c1, (MAG<<1)|0x1, OUTY_H_REG_M, 1, &OUTY_H_REG_M_val, 1, 50);

				OUTZ_L_M_status = HAL_I2C_Mem_Read(&hi2c1, (MAG<<1)|0x1, OUTZ_L_REG_M, 1, &OUTZ_L_REG_M_val, 1, 50);
				OUTZ_H_M_status = HAL_I2C_Mem_Read(&hi2c1, (MAG<<1)|0x1, OUTZ_H_REG_M, 1, &OUTZ_H_REG_M_val, 1, 50);

				// Bitwise operations X, Y, Z registers
				if (OUTX_L_M_status == HAL_OK && OUTX_H_M_status == HAL_OK) {
					OUTX_M_val = OUTX_H_REG_M_val;
					OUTX_M_val <<= 8;
					OUTX_M_val |= OUTX_L_REG_M_val;
				}

				if (OUTY_L_M_status == HAL_OK && OUTY_H_M_status == HAL_OK) {
					OUTY_M_val = OUTY_H_REG_M_val;
					OUTY_M_val <<= 8;
					OUTY_M_val |= OUTY_L_REG_M_val;
				}

				if (OUTZ_L_M_status == HAL_OK && OUTZ_H_M_status == HAL_OK) {
					OUTZ_M_val = OUTZ_H_REG_M_val;
					OUTZ_M_val <<= 8;
					OUTZ_M_val |= OUTZ_L_REG_M_val;
				}
				arr_x_m[i] = OUTX_M_val;
				arr_y_m[i] = OUTY_M_val;
				arr_z_m[i] = OUTZ_M_val;
			}

			/*
			 * Averaging ten samples
			 */
			for (int i=0;i<sample_m;i++) {
				avg_x_m += arr_x_m[i];
				avg_y_m += arr_y_m[i];
				avg_z_m += arr_z_m[i];
			}

			/*
			 * Average Calculation
			 */
			avg_x_m = (avg_x_m / sample_m) * (100.0/65536);
			avg_y_m = (avg_y_m / sample_m) * (100.0/65536);
			avg_z_m = (avg_z_m / sample_m) * (100.0/65536);


			/*
			 * ***** KALMAN FILTER *****
			 */
			KALMAN(avg_x_m, &P_x_m, &U_hat_x_m, &K_x_m);
			KALMAN(avg_y_m, &P_y_m, &U_hat_y_m, &K_y_m);
			KALMAN(avg_z_m, &P_z_m, &U_hat_z_m, &K_z_m);

			/*
			 * Serial Transmit
			 */
			/*
			HAL_UART_Transmit(&huart2, (uint8_t*)MAG_Buffer, sprintf(MAG_Buffer, "% 06.5f,", avg_x_m), 100); // @suppress("Float formatting support")
			HAL_UART_Transmit(&huart2, (uint8_t*)MAG_Buffer, sprintf(MAG_Buffer, "% 06.5f,", avg_y_m), 100); // @suppress("Float formatting support")
			HAL_UART_Transmit(&huart2, (uint8_t*)MAG_Buffer, sprintf(MAG_Buffer, "% 06.5f,", avg_z_m), 100); // @suppress("Float formatting support")
			*/

			/*
			 * Calculate YAW or Heading Angle
			 */
			yaw = atan2f( U_hat_y_m, U_hat_z_m);
			yaw = yaw * 180.0/PI;
		}


		if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == 0 && pushed == 0) {//Blue button is pressed
			//Initialise everything to zero
			angle_count= 0;
			for (int i = 0; i<sizeof(angles)/sizeof(float); i++) {
				angles[i] = 0;//initialise
			}
			offset_avg_count = 0;
			pushed = 1;
			initial_yaw = yaw;
			offset_measure = 1;
			if (MODE == 0) {//For Console
				HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Initialising\n\r"), 100);
				HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Initial Yaw is %f\n\r", initial_yaw), 100);
			}
			steps = 0;
			increase_prev = HAL_GetTick();
			offset = 0;
			x_pos_prev = 0;
			y_pos_prev = 0;
			if (MODE == 0) {//For Console
				HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Stand Still\n\r"), 100);
			}
			HAL_Delay(3000);
			/*
			 * Store Initial Acceleration
			 */
			stationary_x = avg_x_a;
			stationary_y = avg_y_a;
			stationary_z = avg_z_a;
			if (MODE == 0) {//For Console
				/*
				 * Calibration. Takes five stepping motion and averages orientation to zero.
				 */
				HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Stationary x is %f\n\r", stationary_x), 100);
				HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Stationary y is %f\n\r", stationary_y), 100);
				HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Stationary z is %f\n\r", stationary_z), 100);
				HAL_UART_Transmit(&huart2, (uint8_t*)ACC_Buffer, sprintf(ACC_Buffer, "Do 5 steps don't move forward\n\r"), 100);
			}

		} else if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == 1 && pushed == 1) {
			pushed = 0;
		}
	}
}


/*
 * KALMAN FILTER LINEAR
 */
void KALMAN(float U, float *P, float *U_hat, float *K) {
	*K = (*P)*H/(H*(*P)*H+R);
	*U_hat = (*U_hat)+(*K)*(U-H*(*U_hat));
	*P=(1-(*K)*H)*(*P)+Q;
	return;
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
	huart2.Init.BaudRate = 115200;
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
