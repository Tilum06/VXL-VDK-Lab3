#include "traffic_light.h"
#include "main.h"
#include "input_processing.h"
#include "output_display.h"

#define ALL_TRAFFIC_PINS (RED_LED_Pin | YELLOW_LED_Pin | GREEN_LED_Pin | \
                          RED_LED2_Pin | YELLOW_LED2_Pin | GREEN_LED2_Pin)

typedef enum {
    ROAD1_GREEN,
    ROAD1_YELLOW,
    ALL_RED_TO_ROAD2,
    ROAD2_GREEN,
    ROAD2_YELLOW,
    ALL_RED_TO_ROAD1,
    TRAFFIC_PHASE_COUNT
} TrafficPhase;

static uint8_t red_duration_s = 5u;
static uint8_t yellow_duration_s = 2u;
static uint8_t green_duration_s = 3u;
static InputMode active_mode;
static uint32_t phase_tick_ms;
static uint32_t phase_duration_ms[TRAFFIC_PHASE_COUNT];
static uint32_t cycle_ms;
static TrafficPhase traffic_phase;
static uint16_t blink_pins;
static uint8_t last_display1, last_display2;

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

static void apply_traffic_phase(void)
{
    static const uint16_t phase_pins[TRAFFIC_PHASE_COUNT] = {
        GREEN_LED_Pin | RED_LED2_Pin,
        YELLOW_LED_Pin | RED_LED2_Pin,
        RED_LED_Pin | RED_LED2_Pin,
        RED_LED_Pin | GREEN_LED2_Pin,
        RED_LED_Pin | YELLOW_LED2_Pin,
        RED_LED_Pin | RED_LED2_Pin
    };
    set_traffic_leds(phase_pins[traffic_phase]);
}

static void start_normal_traffic(uint32_t now_ms)
{
    uint32_t green_ms = (uint32_t)green_duration_s * 1000u;
    uint32_t yellow_ms = (uint32_t)yellow_duration_s * 1000u;
    uint32_t extra_ms = ((uint32_t)red_duration_s - green_duration_s -
                         yellow_duration_s) * 1000u;
    phase_duration_ms[ROAD1_GREEN] = phase_duration_ms[ROAD2_GREEN] = green_ms;
    phase_duration_ms[ROAD1_YELLOW] = phase_duration_ms[ROAD2_YELLOW] = yellow_ms;
    phase_duration_ms[ALL_RED_TO_ROAD2] = extra_ms / 2u;
    phase_duration_ms[ALL_RED_TO_ROAD1] = extra_ms - extra_ms / 2u;
    cycle_ms = green_ms * 2u + yellow_ms * 2u + extra_ms;
    traffic_phase = ROAD1_GREEN;
    phase_tick_ms = now_ms;
    last_display1 = last_display2 = 0xffu;
    apply_traffic_phase();
}

static void display_countdown(uint32_t elapsed_ms)
{
    uint32_t remaining = phase_duration_ms[traffic_phase] - elapsed_ms;
    uint32_t road1 = remaining, road2 = remaining;
    switch (traffic_phase)
    {
    case ROAD1_GREEN:
        road2 += phase_duration_ms[ROAD1_YELLOW] + phase_duration_ms[ALL_RED_TO_ROAD2];
        break;
    case ROAD1_YELLOW:
        road2 += phase_duration_ms[ALL_RED_TO_ROAD2];
        break;
    case ALL_RED_TO_ROAD2:
        road1 += phase_duration_ms[ROAD2_GREEN] + phase_duration_ms[ROAD2_YELLOW] +
                 phase_duration_ms[ALL_RED_TO_ROAD1];
        break;
    case ROAD2_GREEN:
        road1 += phase_duration_ms[ROAD2_YELLOW] + phase_duration_ms[ALL_RED_TO_ROAD1];
        break;
    case ROAD2_YELLOW:
        road1 += phase_duration_ms[ALL_RED_TO_ROAD1];
        break;
    case ALL_RED_TO_ROAD1:
        road2 += phase_duration_ms[ROAD1_GREEN] + phase_duration_ms[ROAD1_YELLOW] +
                 phase_duration_ms[ALL_RED_TO_ROAD2];
        break;
    default:
        break;
    }
    /* Ceil: show 01 until the actual color changes, including half seconds. */
    uint8_t first = (uint8_t)((road1 + 999u) / 1000u);
    uint8_t second = (uint8_t)((road2 + 999u) / 1000u);
    if ((first != last_display1) || (second != last_display2))
    {
        output_display_set_pairs(first, second);
        last_display1 = first;
        last_display2 = second;
    }
}

static void process_normal_traffic(uint32_t now_ms)
{
    uint32_t elapsed_ms = now_ms - phase_tick_ms;
    /* Skip whole cycles without issuing obsolete GPIO transitions. */
    uint32_t cycles = elapsed_ms / cycle_ms;
    phase_tick_ms += cycles * cycle_ms;
    elapsed_ms %= cycle_ms;
    TrafficPhase previous_phase = traffic_phase;
    while (elapsed_ms >= phase_duration_ms[traffic_phase])
    {
        uint32_t duration = phase_duration_ms[traffic_phase];
        elapsed_ms -= duration;
        phase_tick_ms += duration;
        traffic_phase = (TrafficPhase)((traffic_phase + 1u) % TRAFFIC_PHASE_COUNT);
    }
    if (traffic_phase != previous_phase) apply_traffic_phase();
    display_countdown(elapsed_ms);
}

static void enter_mode(InputMode mode, uint32_t now_ms)
{
    active_mode = mode;
    phase_tick_ms = now_ms;
    blink_pins = 0u;
    if (mode == INPUT_MODE_NORMAL)
    {
        start_normal_traffic(now_ms);
        return;
    }
    if (mode == INPUT_MODE_RED) blink_pins = RED_LED_Pin | RED_LED2_Pin;
    else if (mode == INPUT_MODE_YELLOW) blink_pins = YELLOW_LED_Pin | YELLOW_LED2_Pin;
    else if (mode == INPUT_MODE_GREEN) blink_pins = GREEN_LED_Pin | GREEN_LED2_Pin;
    set_traffic_leds(blink_pins);
}

void traffic_light_init(void)
{
    set_traffic_leds(0u);
    active_mode = (InputMode)0;
}

void traffic_light_process(void)
{
    uint32_t now_ms = HAL_GetTick();
    InputMode mode = input_mode;
    if (mode != active_mode) enter_mode(mode, now_ms);
    if (mode == INPUT_MODE_NORMAL)
    {
        process_normal_traffic(now_ms);
        return;
    }
    uint32_t half_periods = (uint32_t)(now_ms - phase_tick_ms) /
                            TRAFFIC_LIGHT_BLINK_HALF_PERIOD_MS;
    phase_tick_ms += half_periods * TRAFFIC_LIGHT_BLINK_HALF_PERIOD_MS;
    if (((half_periods & 1u) != 0u) && (blink_pins != 0u))
        HAL_GPIO_TogglePin(GPIOA, blink_pins);
}
