#include "traffic_light.h"
#include "main.h"

#define ALL_TRAFFIC_PINS (RED_LED_Pin | YELLOW_LED_Pin | GREEN_LED_Pin | \
                          RED_LED2_Pin | YELLOW_LED2_Pin | GREEN_LED2_Pin)

static uint8_t red_duration_s = 5u;
static uint8_t yellow_duration_s = 2u;
static uint8_t green_duration_s = 3u;

static uint8_t durations_valid(uint8_t red, uint8_t yellow, uint8_t green)
{
    return (red >= 1u) && (red <= 99u) &&
           (yellow >= 1u) && (yellow <= 99u) &&
           (green >= 1u) && (green <= 99u) &&
           ((uint16_t)red >= (uint16_t)yellow + green);
}

uint8_t traffic_light_get_red_duration_s(void) { return red_duration_s; }
uint8_t traffic_light_get_yellow_duration_s(void) { return yellow_duration_s; }
uint8_t traffic_light_get_green_duration_s(void) { return green_duration_s; }

uint8_t traffic_light_set_red_duration_s(uint8_t duration_s)
{
    if (!durations_valid(duration_s, yellow_duration_s, green_duration_s)) return 0u;
    red_duration_s = duration_s;
    return 1u;
}

uint8_t traffic_light_set_yellow_duration_s(uint8_t duration_s)
{
    if (!durations_valid(red_duration_s, duration_s, green_duration_s)) return 0u;
    yellow_duration_s = duration_s;
    return 1u;
}

uint8_t traffic_light_set_green_duration_s(uint8_t duration_s)
{
    if (!durations_valid(red_duration_s, yellow_duration_s, duration_s)) return 0u;
    green_duration_s = duration_s;
    return 1u;
}

static void set_traffic_leds(uint16_t on_pins)
{
    /* Break before make: old colors off before any new color turns on. */
    HAL_GPIO_WritePin(GPIOA, ALL_TRAFFIC_PINS, LED_OFF);
    if (on_pins != 0u) HAL_GPIO_WritePin(GPIOA, on_pins, LED_ON);
}

static uint16_t color_pins(TrafficColor color)
{
    switch (color)
    {
    case TRAFFIC_COLOR_YELLOW:
        return YELLOW_LED_Pin | YELLOW_LED2_Pin;
    case TRAFFIC_COLOR_GREEN:
        return GREEN_LED_Pin | GREEN_LED2_Pin;
    case TRAFFIC_COLOR_RED:
    default:
        return RED_LED_Pin | RED_LED2_Pin;
    }
}

void traffic_light_init(void)
{
    set_traffic_leds(0u);
}

void traffic_light_set_roads(TrafficColor road1, TrafficColor road2)
{
    const uint16_t road1_pins = RED_LED_Pin | YELLOW_LED_Pin | GREEN_LED_Pin;
    const uint16_t road2_pins = RED_LED2_Pin | YELLOW_LED2_Pin | GREEN_LED2_Pin;
    /* Fail safe even if a caller accidentally requests opposing greens. */
    if ((road1 == TRAFFIC_COLOR_GREEN) && (road2 == TRAFFIC_COLOR_GREEN))
    {
        road1 = road2 = TRAFFIC_COLOR_RED;
    }
    set_traffic_leds((color_pins(road1) & road1_pins) |
                     (color_pins(road2) & road2_pins));
}

void traffic_light_set_blink_color(TrafficColor color)
{
    set_traffic_leds(color_pins(color));
}

void traffic_light_toggle_color(TrafficColor color)
{
    HAL_GPIO_TogglePin(GPIOA, color_pins(color));
}
