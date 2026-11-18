
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "heliostat_logic.h"
#include "heliostat_devices.h"
#include "platform_io.h"
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

  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  /* USER CODE END SysInit */

  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();

  /* USER CODE BEGIN 2 */

  platform_uart_write("\r\n=== STM32 HELIOSTAT ===\r\n");
  platform_uart_write("System initialization\r\n");

  if (HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK)
  {
      Error_Handler();
  }

  if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1) != HAL_OK)
  {
      Error_Handler();
  }

  if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2) != HAL_OK)
  {
      Error_Handler();
  }

  HeliostatState heliostat;
  heliostat_init(&heliostat);

  device_servos_set(
      heliostat.pulse_x,
      heliostat.pulse_y
  );

  uint8_t oled_found = device_oled_detect();

  platform_uart_write(
      oled_found ? "OLED: FOUND\r\n" : "OLED: NOT FOUND\r\n"
  );

  uint8_t previous_button = device_button_level();

  uint32_t last_button_time = 0;
  uint32_t last_update = 0;
  uint32_t tick = 0;

  platform_uart_write(
      "\r\n=== HELIOSTAT SELF-TEST ===\r\n"
  );

  char test_msg[128];

  uint16_t test_adc_x = device_light_read_x();
  uint16_t test_adc_y = device_light_read_y();

  snprintf(
      test_msg,
      sizeof(test_msg),
      "SELFTEST,ADC_X,SAMPLE,%u\r\n",
      (unsigned int)test_adc_x
  );
  platform_uart_write(test_msg);

  snprintf(
      test_msg,
      sizeof(test_msg),
      "SELFTEST,ADC_Y,SAMPLE,%u\r\n",
      (unsigned int)test_adc_y
  );
  platform_uart_write(test_msg);

  platform_uart_write("SELFTEST,PWM_X,STARTED\r\n");
  platform_uart_write("SELFTEST,PWM_Y,STARTED\r\n");

  platform_uart_write(
      oled_found
          ? "SELFTEST,OLED,ACK\r\n"
          : "SELFTEST,OLED,NO_ACK\r\n"
  );

  uint8_t button_state = device_button_level();

  snprintf(
      test_msg,
      sizeof(test_msg),
      "SELFTEST,BUTTON,LEVEL,%u\r\n",
      (unsigned int)button_state
  );
  platform_uart_write(test_msg);

  platform_uart_write("SELFTEST,END\r\n\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    uint32_t now = HAL_GetTick();

    uint8_t button = device_button_level();

    if (previous_button && !button &&
        now - last_button_time >= 200U)
    {
        heliostat_toggle_mode(&heliostat);
        last_button_time = now;

        platform_uart_write(
            heliostat.mode == HELIOSTAT_AUTO
                ? "MODE: AUTO\r\n"
                : "MODE: STANDBY\r\n"
        );
    }

    previous_button = button;

    if (now - last_update >= 1000U)
    {
        last_update = now;

        uint16_t light_x = device_light_read_x();
        uint16_t light_y = device_light_read_y();

        heliostat_update(
            &heliostat,
            light_x,
            light_y
        );

        if (heliostat.mode == HELIOSTAT_AUTO)
        {
            device_servos_set(
                heliostat.pulse_x,
                heliostat.pulse_y
            );
        }

        char buffer[160];

        snprintf(
            buffer,
            sizeof(buffer),
            "tick %lu | X=%u Y=%u | "
            "angleX=%u angleY=%u | mode=%s\r\n",
            (unsigned long)++tick,
            (unsigned int)heliostat.adc_x,
            (unsigned int)heliostat.adc_y,
            (unsigned int)heliostat.angle_x,
            (unsigned int)heliostat.angle_y,
            heliostat.mode == HELIOSTAT_AUTO
                ? "AUTO"
                : "STANDBY"
        );

        platform_uart_write(buffer);
    }

    HAL_Delay(10);
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

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue =
      RCC_HSICALIBRATION_DEFAULT;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
      Error_Handler();
  }

  RCC_ClkInitStruct.ClockType =
      RCC_CLOCKTYPE_HCLK |
      RCC_CLOCKTYPE_SYSCLK |
      RCC_CLOCKTYPE_PCLK1;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(
          &RCC_ClkInitStruct,
          FLASH_LATENCY_1
      ) != HAL_OK)
  {
      Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();

  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif
