/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "lcd1602_i2c.h"
#include "adc.h"
#include "servo.h"
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// Tracking system sensitivity settings
#define SENSITIVITY 5 // Sensitivity (minimum difference for reaction)
#define STEP_SIZE 5  // Servo angle step size (degrees)

// Structure for storing data from one sensor
typedef struct {
    uint32_t raw;
    uint32_t percent;
    uint32_t previous_percent;
    uint8_t stable;
} SensorData;

// Array for 4 photosensors (photoresistors)
SensorData sensors[4] = {
    {0, 0, 0, 0},  // A3
    {0, 0, 0, 0},  // A4
    {0, 0, 0, 0},  // A5
    {0, 0, 0, 0}   // A6
};

// Buffers for LCD output
char lcd_buffer[17];

uint32_t maximum = 0; // Maximum after scanning
uint32_t maximum_old = 0; // A variable for finding the maximum
uint32_t max_angle_y = 0; // The y-coordinate of the maximum
uint32_t max_angle_x = 0; // The x-coordinate of the maximum
uint32_t max_sensor0 = 0; // Maximum sensor values
uint32_t max_sensor1 = 0;
uint32_t max_sensor2 = 0;
uint32_t max_sensor3 = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
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
  MX_I2C2_Init();
  MX_TIM2_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

char buffer[17];  // LCD string buffer (max 16 characters + \0)
	
Servo_Init();
lcd_Init(&hi2c2);

// Display welcome message on LCD
lcd_SendString("Photo Sensors");
lcd_SetCursor(1, 0);
lcd_SendString("A3 A4 A5 A6");
HAL_Delay(2000);
lcd_Clear();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
  while (1)
  {
    /* USER CODE END WHILE */
		
		// ========== STAGE 1: Finding position with maximum brightness ==========
		
    // Check ADC
	  uint32_t adc_values[4];
    adc_values[0] = ADC_Read_Filtered(ADC_CHANNEL_3);  // A3
    adc_values[1] = ADC_Read_Filtered(ADC_CHANNEL_4);  // A4
    adc_values[2] = ADC_Read_Filtered(ADC_CHANNEL_5);  // A5
    adc_values[3] = ADC_Read_Filtered(ADC_CHANNEL_6);  // A6
    
    // Convert from ADC to %
    for (int i = 0; i < 4; i++) {
        sensors[i].raw = adc_values[i];
        sensors[i].percent = ADC_To_Percent(adc_values[i]);  // 0-4095 -> 0-100
        // Check overflow
        if (sensors[i].percent > 100) sensors[i].percent = 100;
        
        // Check stability
        if (abs((int)sensors[i].percent - (int)sensors[i].previous_percent) < 3) {
            sensors[i].stable = 1;
        } else {
            sensors[i].stable = 0;
        }
        sensors[i].previous_percent = sensors[i].percent;
    }
    
    // Output to LCD
    
    // A3 and A4
    lcd_SetCursor(0, 0);
    sprintf(lcd_buffer, "A3:%3d%% A4:%3d%%", sensors[0].percent, sensors[1].percent);
    lcd_SendString(lcd_buffer);
    
    // A5 and A6
    lcd_SetCursor(1, 0);
    sprintf(lcd_buffer, "A5:%3d%% A6:%3d%%", sensors[2].percent, sensors[3].percent);
    lcd_SendString(lcd_buffer);
    
    // Moving of servo
    for (uint16_t angle_y = 0; angle_y <= 180; angle_y += 20) {
    
    // Set Y servo angle (vertical position)
    Servo_SetAngle(3, angle_y);
    HAL_Delay(100);  // Delay for Y servo movement to complete
    
    // Move along X axis (horizontal, servo 2) from 0 to 180 in 10° steps
    for (uint16_t angle_x = 0; angle_x <= 180; angle_x += 10) {
        
        Servo_SetAngle(2, angle_x);
        HAL_Delay(10);  // Short delay for stabilization
        
        // === Measure brightness at current position ===
        uint32_t tmp3 = ADC_Read_Filtered(ADC_CHANNEL_3);
        uint32_t tmp4 = ADC_Read_Filtered(ADC_CHANNEL_4);
        uint32_t tmp5 = ADC_Read_Filtered(ADC_CHANNEL_5);
        uint32_t tmp6 = ADC_Read_Filtered(ADC_CHANNEL_6);
        
        sensors[0].percent = ADC_To_Percent(tmp3);
        sensors[1].percent = ADC_To_Percent(tmp4);
        sensors[2].percent = ADC_To_Percent(tmp5);
        sensors[3].percent = ADC_To_Percent(tmp6);
        
        // Limit to 100%
        for (int i = 0; i < 4; i++) {
            if (sensors[i].percent > 100) sensors[i].percent = 100;
        }
        
        // Update LCD every 20° of X rotation (for visual monitoring)
        if (angle_x % 20 == 0) {
    // A3 and A4
    lcd_SetCursor(0, 0);
    sprintf(lcd_buffer, "A3:%3d%% A4:%3d%%", sensors[0].percent, sensors[1].percent);
    lcd_SendString(lcd_buffer);
    
    // A5 and A6
    lcd_SetCursor(1, 0);
    sprintf(lcd_buffer, "A5:%3d%% A6:%3d%%", sensors[2].percent, sensors[3].percent);
    lcd_SendString(lcd_buffer);
}
				// Calculate total brightness (sum of all 4 sensors)
				maximum = sensors[0].percent + sensors[1].percent + sensors[2].percent + sensors[3].percent;

        // If new maximum sum found - remember angles and readings
        if(maximum > maximum_old) {
					maximum_old = maximum;
					max_angle_x = angle_x;
					max_angle_y = angle_y;
					max_sensor0 = sensors[0].percent;
					max_sensor1 = sensors[1].percent;
					max_sensor2 = sensors[2].percent;
					max_sensor3 = sensors[3].percent;
				}
    }
    
    // Pause after completing X scanning before changing Y
    HAL_Delay(300);
}
		
// === Set servos to position of maximum brightness ===
Servo_SetAngle(2, max_angle_x);
HAL_Delay(100);
Servo_SetAngle(3, max_angle_y);
HAL_Delay(100);

// Display found maximum on LCD
lcd_SetCursor(0, 0);
    sprintf(lcd_buffer, "A3:%3d%% A4:%3d%%", max_sensor0, max_sensor1);
    lcd_SendString(lcd_buffer);
    
    lcd_SetCursor(1, 0);
    sprintf(lcd_buffer, "A5:%3d%% A6:%3d%%", max_sensor2, max_sensor3);
    lcd_SendString(lcd_buffer);
break; // Exit scanning loop and move to tracking mode
}

    /* USER CODE BEGIN 3 */

// ========== STAGE 2: Main light source tracking loop ==========

uint8_t current_x = max_angle_x; // Current X angle 
uint8_t current_y = max_angle_y; // Current Y angle 

  while(1) {

    uint32_t tmp3 = ADC_Read_Filtered(ADC_CHANNEL_3);
    uint32_t tmp4 = ADC_Read_Filtered(ADC_CHANNEL_4);
    uint32_t tmp5 = ADC_Read_Filtered(ADC_CHANNEL_5);
    uint32_t tmp6 = ADC_Read_Filtered(ADC_CHANNEL_6);
    
    // Convert to percent
    sensors[0].percent = (tmp3 / 41 > 100) ? 100 : ADC_To_Percent(tmp3);
    sensors[1].percent = (tmp4 / 41 > 100) ? 100 : ADC_To_Percent(tmp4);
    sensors[2].percent = (tmp5 / 41 > 100) ? 100 : ADC_To_Percent(tmp5);
    sensors[3].percent = (tmp6 / 41 > 100) ? 100 : ADC_To_Percent(tmp6);
    
    // === Calculate group indicators for direction detection ===
    
    // Top sensors: A3 + A6
    uint8_t top_sensors = (sensors[0].percent + sensors[3].percent) / 2;
    
    // Bottom sensors: A4 + A5 
    uint8_t bottom_sensors = (sensors[1].percent + sensors[2].percent) / 2;
    
    // Left sensors: A4 + A3
    uint8_t left_sensors = (sensors[1].percent + sensors[0].percent) / 2;
    
    // Right sensors: A5 + A6
    uint8_t right_sensors = (sensors[2].percent + sensors[3].percent) / 2;
    
    // Compare left and right sides
    if (left_sensors > right_sensors + SENSITIVITY) {
        // Light is brighter on the left - turn left
			if(current_y > 0){
            current_y -= STEP_SIZE;
            Servo_SetAngle(3, current_y);
            HAL_Delay(100);
			}
        
    } 
    if (right_sensors > left_sensors + SENSITIVITY) {
        // Light is brighter on the right - turn right
			if(current_y < 180){
            current_y += STEP_SIZE;
            Servo_SetAngle(3, current_y);
            HAL_Delay(100);
			}

    }
    
        // Compare top and bottom sides
        if (top_sensors > bottom_sensors + SENSITIVITY) {
            // Light is brighter from above - tilt up
          if(current_x < 180) {
                current_x += STEP_SIZE;
                Servo_SetAngle(2, current_x);
                HAL_Delay(100);
					}
            
        } 
        if (bottom_sensors > top_sensors + SENSITIVITY) {
            // Light is brighter from below - tilt down
           if(current_x > 0) {
                current_x -= STEP_SIZE;
                Servo_SetAngle(2, current_x);
                HAL_Delay(100);
					 }
            
        }
    
    // === Display information on LCD ===
    lcd_Clear();
    lcd_SetCursor(0, 0);
    sprintf(lcd_buffer, "Y:%3d L%2d R%2d", current_y, left_sensors, right_sensors);
    lcd_SendString(lcd_buffer);
    
    lcd_SetCursor(1, 0);
    sprintf(lcd_buffer, "X:%3d U%2d D%2d", current_x, top_sensors, bottom_sensors);
    lcd_SendString(lcd_buffer);
    
   
    
    // Delay between tracking loop iterations
    HAL_Delay(150);

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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
#ifdef USE_FULL_ASSERT
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
