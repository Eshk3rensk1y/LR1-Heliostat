
#include "heliostat_logic.h"

static uint16_t clamp_adc(uint16_t value)
{
    return value > 4095U ? 4095U : value;
}

uint16_t heliostat_adc_to_angle(uint16_t adc)
{
    uint32_t value = clamp_adc(adc);
    return (uint16_t)((value * 180U) / 4095U);
}

uint16_t heliostat_adc_to_pulse(uint16_t adc)
{
    uint32_t value = clamp_adc(adc);
    return (uint16_t)(1000U + (value * 1000U) / 4095U);
}

void heliostat_init(HeliostatState *state)
{
    if (state == 0)
        return;

    state->adc_x = 2048;
    state->adc_y = 2048;

    state->angle_x = 90;
    state->angle_y = 90;

    state->pulse_x = 1500;
    state->pulse_y = 1500;

    state->mode = HELIOSTAT_AUTO;
}

void heliostat_update(
    HeliostatState *state,
    uint16_t adc_x,
    uint16_t adc_y
)
{
    if (state == 0)
        return;

    state->adc_x = clamp_adc(adc_x);
    state->adc_y = clamp_adc(adc_y);

    state->angle_x =
        heliostat_adc_to_angle(state->adc_x);

    state->angle_y =
        heliostat_adc_to_angle(state->adc_y);

    if (state->mode == HELIOSTAT_AUTO)
    {
        state->pulse_x =
            heliostat_adc_to_pulse(state->adc_x);

        state->pulse_y =
            heliostat_adc_to_pulse(state->adc_y);
    }
}

void heliostat_toggle_mode(HeliostatState *state)
{
    if (state == 0)
        return;

    state->mode =
        state->mode == HELIOSTAT_AUTO
        ? HELIOSTAT_STANDBY
        : HELIOSTAT_AUTO;
}
