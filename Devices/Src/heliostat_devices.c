
#include "heliostat_devices.h"
#include "platform_io.h"

#define LIGHT_X_CHANNEL 0U
#define LIGHT_Y_CHANNEL 1U

uint16_t device_light_read_x(void)
{
    return platform_adc_read(LIGHT_X_CHANNEL);
}

uint16_t device_light_read_y(void)
{
    return platform_adc_read(LIGHT_Y_CHANNEL);
}

void device_servos_set(uint16_t x, uint16_t y)
{
    platform_servo_set(x, y);
}

uint8_t device_oled_detect(void)
{
    return platform_oled_probe();
}

uint8_t device_button_level(void)
{
    return platform_button_level();
}
