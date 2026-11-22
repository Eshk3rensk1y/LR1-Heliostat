
#ifndef HELIOSTAT_DEVICES_H
#define HELIOSTAT_DEVICES_H

#include <stdint.h>

uint16_t device_light_read_x(void);
uint16_t device_light_read_y(void);

void device_servos_set(uint16_t x, uint16_t y);

uint8_t device_oled_detect(void);

uint8_t device_button_level(void);

#endif
