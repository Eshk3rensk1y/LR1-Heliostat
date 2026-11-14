
#include <assert.h>
#include <stdio.h>

#include "heliostat_logic.h"

static void test_adc_conversion(void)
{
    assert(heliostat_adc_to_angle(0) == 0);
    assert(heliostat_adc_to_angle(4095) == 180);

    assert(heliostat_adc_to_pulse(0) == 1000);
    assert(heliostat_adc_to_pulse(4095) == 2000);

    assert(heliostat_adc_to_pulse(2048) >= 1499);
    assert(heliostat_adc_to_pulse(2048) <= 1501);

    assert(heliostat_adc_to_angle(5000) == 180);
}

static void test_auto_mode(void)
{
    HeliostatState state;

    heliostat_init(&state);

    assert(state.mode == HELIOSTAT_AUTO);

    heliostat_update(&state, 0, 4095);

    assert(state.angle_x == 0);
    assert(state.angle_y == 180);

    assert(state.pulse_x == 1000);
    assert(state.pulse_y == 2000);
}

static void test_standby_mode(void)
{
    HeliostatState state;

    heliostat_init(&state);

    heliostat_toggle_mode(&state);

    assert(state.mode == HELIOSTAT_STANDBY);

    heliostat_update(&state, 0, 4095);

    assert(state.adc_x == 0);
    assert(state.adc_y == 4095);

    assert(state.pulse_x == 1500);
    assert(state.pulse_y == 1500);

    heliostat_toggle_mode(&state);

    assert(state.mode == HELIOSTAT_AUTO);

    heliostat_update(&state, 0, 4095);

    assert(state.pulse_x == 1000);
    assert(state.pulse_y == 2000);
}

int main(void)
{
    test_adc_conversion();
    test_auto_mode();
    test_standby_mode();

    printf("PASS: ADC conversion\n");
    printf("PASS: AUTO mode\n");
    printf("PASS: STANDBY mode\n");
    printf("ALL TESTS PASSED\n");

    return 0;
}
