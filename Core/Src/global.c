#include "global.h"
InputMode input_mode = INPUT_MODE_NORMAL;
TrafficPhase traffic_phase = ROAD1_GREEN;
ManualState manual_state = MAN_ROAD1_GREEN;
uint8_t red_time = 5, yellow_time = 2, green_time = 3;
uint8_t set_pressed, save_pressed;

uint8_t durations_valid(uint8_t red, uint8_t yellow, uint8_t green)
{
    return red >= 1 && red <= 99 && yellow >= 1 && yellow <= 99 &&
           green >= 1 && green <= 99 && red >= (uint16_t)yellow + green;
}

uint32_t all_red_ms(void)
{
    /* Two equal clearances per cycle: each road stays red for exactly RED. */
    return (uint32_t)(red_time - green_time - yellow_time) * 500u;
}
