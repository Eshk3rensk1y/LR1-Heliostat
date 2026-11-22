
#ifndef PLATFORM_IO_H
#define PLATFORM_IO_H

#include <stdint.h>

void platform_uart_write(const char *text);

uint16_t platform_adc_read(uint32_t channel);

void platform_servo_set(uint16_t x, uint16_t y);

uint8_t platform_oled_probe(void);

uint8_t platform_button_level(void);

#endif
