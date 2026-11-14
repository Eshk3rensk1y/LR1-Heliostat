
#ifndef HELIOSTAT_LOGIC_H
#define HELIOSTAT_LOGIC_H

#include <stdint.h>

typedef enum {
    HELIOSTAT_AUTO = 0,
    HELIOSTAT_STANDBY = 1
} HeliostatMode;

typedef struct {
    uint16_t adc_x;
    uint16_t adc_y;
    uint16_t angle_x;
    uint16_t angle_y;
    uint16_t pulse_x;
    uint16_t pulse_y;
    HeliostatMode mode;
} HeliostatState;

uint16_t heliostat_adc_to_angle(uint16_t adc);
uint16_t heliostat_adc_to_pulse(uint16_t adc);

void heliostat_init(HeliostatState *state);

void heliostat_update(
    HeliostatState *state,
    uint16_t adc_x,
    uint16_t adc_y
);

void heliostat_toggle_mode(HeliostatState *state);

#endif
