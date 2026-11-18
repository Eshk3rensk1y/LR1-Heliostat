
#include "platform_io.h"

#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

#include <string.h>

void platform_uart_write(const char *text)
{
    if (text == NULL)
        return;

    HAL_UART_Transmit(
        &huart2,
        (uint8_t *)text,
        (uint16_t)strlen(text),
        1000
    );
}

uint16_t platform_adc_read(uint32_t channel)
{
    ADC_ChannelConfTypeDef config = {0};

    if (channel > 1U)
        return 0;

    config.Channel =
        channel == 0U ? ADC_CHANNEL_0 : ADC_CHANNEL_1;

    config.Rank = ADC_REGULAR_RANK_1;
    config.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;

    if (HAL_ADC_ConfigChannel(&hadc1, &config) != HAL_OK)
        return 0;

    if (HAL_ADC_Start(&hadc1) != HAL_OK)
        return 0;

    uint16_t value = 0;

    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
        value = (uint16_t)HAL_ADC_GetValue(&hadc1);
    }

    HAL_ADC_Stop(&hadc1);

    return value;
}

void platform_servo_set(uint16_t x, uint16_t y)
{
    __HAL_TIM_SET_COMPARE(
        &htim3,
        TIM_CHANNEL_1,
        x
    );

    __HAL_TIM_SET_COMPARE(
        &htim3,
        TIM_CHANNEL_2,
        y
    );
}

uint8_t platform_oled_probe(void)
{
    return HAL_I2C_IsDeviceReady(
        &hi2c1,
        0x3CU << 1,
        2,
        100
    ) == HAL_OK;
}

uint8_t platform_button_level(void)
{
    return HAL_GPIO_ReadPin(
        GPIOA,
        GPIO_PIN_8
    ) == GPIO_PIN_SET;
}
