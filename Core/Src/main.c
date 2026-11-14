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
#include <string.h>
#include "heliostat_logic.h"

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

static void uart_print(const char *text)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)text,
                      (uint16_t)strlen(text), 1000);
}

static uint16_t read_adc(uint32_t channel)
{
    ADC_ChannelConfTypeDef config = {0};

    config.Channel = channel;
    config.Rank = ADC_REGULAR_RANK_1;
    config.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;

    if (HAL_ADC_ConfigChannel(&hadc1, &config) != HAL_OK)
        return 0;

    if (HAL_ADC_Start(&hadc1) != HAL_OK)
        return 0;

    uint16_t value = 0;

    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
        value = (uint16_t)HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);
    return value;
}


static void set_servo_positions(uint16_t x, uint16_t y)
{
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, x);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, y);
}

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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  
/* USER CODE BEGIN 2 */

uart_print("\r\n=== STM32 HELIOSTAT ===\r\n");
uart_print("System initialization\r\n");

HAL_ADCEx_Calibration_Start(&hadc1);

if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1) != HAL_OK)
    Error_Handler();

if (HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2) != HAL_OK)
    Error_Handler();


HeliostatState heliostat;
heliostat_init(&heliostat);

set_servo_positions(
    heliostat.pulse_x,
    heliostat.pulse_y
);


uint8_t oled_found =
    HAL_I2C_IsDeviceReady(&hi2c1, 0x3C << 1, 2, 100) == HAL_OK;

uart_print(oled_found ? "OLED: FOUND\r\n" : "OLED: NOT FOUND\r\n");

uint8_t previous_button = 1;
uint32_t last_button_time = 0;
uint32_t last_update = 0;
uint32_t tick = 0;

uart_print("\r\n=== HELIOSTAT SELF-TEST ===\r\n");

char test_msg[128];

uint16_t test_adc_x = read_adc(ADC_CHANNEL_0);
uint16_t test_adc_y = read_adc(ADC_CHANNEL_1);

snprintf(test_msg, sizeof(test_msg),
          "SELFTEST,ADC_X,SAMPLE,%u\r\n", test_adc_x);
uart_print(test_msg);

snprintf(test_msg, sizeof(test_msg),
          "SELFTEST,ADC_Y,SAMPLE,%u\r\n", test_adc_y);
uart_print(test_msg);

uart_print("SELFTEST,PWM_X,STARTED\r\n");
uart_print("SELFTEST,PWM_Y,STARTED\r\n");

uart_print(oled_found ?
            "SELFTEST,OLED,ACK\r\n" :
            "SELFTEST,OLED,NO_ACK\r\n");

GPIO_PinState button_state =
    HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8);

snprintf(test_msg, sizeof(test_msg),
          "SELFTEST,BUTTON,LEVEL,%u\r\n",
          button_state == GPIO_PIN_SET ? 1U : 0U);
uart_print(test_msg);

uart_print("SELFTEST,END\r\n\r\n");

/* USER CODE END 2 */


  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    
    uint32_t now = HAL_GetTick();

    uint8_t button =
        HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_SET;

    if (previous_button && !button &&
        now - last_button_time >= 200U)
    {
        heliostat_toggle_mode(&heliostat);
        last_button_time = now;

        uart_print(heliostat.mode == HELIOSTAT_AUTO ?
                   "MODE: AUTO\r\n" :
                   "MODE: STANDBY\r\n");
    }

    previous_button = button;

    if (now - last_update >= 1000U)
    {
        last_update = now;


        uint16_t light_x = read_adc(ADC_CHANNEL_0);
        uint16_t light_y = read_adc(ADC_CHANNEL_1);

        heliostat_update(
            &heliostat,
            light_x,
            light_y
        );

        if (heliostat.mode == HELIOSTAT_AUTO)
        {
            set_servo_positions(
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

        uart_print(buffer);

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;

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
