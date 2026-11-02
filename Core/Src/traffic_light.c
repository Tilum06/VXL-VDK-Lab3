#include "traffic_light.h"
#include "main.h"
#include "input_processing.h"

#define ALL_TRAFFIC_PINS (RED_LED_Pin | YELLOW_LED_Pin | GREEN_LED_Pin | \
                          RED_LED2_Pin | YELLOW_LED2_Pin | GREEN_LED2_Pin)

static uint32_t blink_tick_ms;

static void set_traffic_leds(uint16_t on_pins)
{
    HAL_GPIO_WritePin(GPIOA, ALL_TRAFFIC_PINS, LED_OFF);
    if (on_pins != 0u)
    {
        HAL_GPIO_WritePin(GPIOA, on_pins, LED_ON);
    }
}

#if TRAFFIC_LED_TEST
static uint8_t test_phase;
static const uint16_t test_pins[4] = {
    RED_LED_Pin | RED_LED2_Pin,
    YELLOW_LED_Pin | YELLOW_LED2_Pin,
    GREEN_LED_Pin | GREEN_LED2_Pin,
    0u
};
#define TRAFFIC_LED_TEST_STEP_MS 2000u
#else
static InputMode blink_mode;
static uint16_t blink_pins;

static void select_blink_mode(InputMode mode, uint32_t now_ms)
{
    /* User-confirmed wiring: anode to +3.3 V, cathode to GPIO. */
    blink_mode = mode;
    blink_tick_ms = now_ms;

    switch (mode)
    {
    case INPUT_MODE_NORMAL:
        /* Retain Ex4's single PA5 blink until the normal traffic FSM exists. */
        blink_pins = RED_LED_Pin;
        break;
    case INPUT_MODE_RED:
        blink_pins = RED_LED_Pin | RED_LED2_Pin;
        break;
    case INPUT_MODE_YELLOW:
        blink_pins = YELLOW_LED_Pin | YELLOW_LED2_Pin;
        break;
    case INPUT_MODE_GREEN:
        blink_pins = GREEN_LED_Pin | GREEN_LED2_Pin;
        break;
    default:
        blink_pins = 0u;
        break;
    }
    /* Start each selected mode with a complete 250 ms ON phase. */
    set_traffic_leds(blink_pins);
}
#endif

void traffic_light_init(void)
{
    set_traffic_leds(0u);
    blink_tick_ms = HAL_GetTick();
#if TRAFFIC_LED_TEST
    /* First foreground call starts red; init leaves all six pins HIGH. */
    test_phase = 4u;
#else
    blink_mode = (InputMode)0;
    blink_pins = 0u;
#endif
}

void traffic_light_process(void)
{
    /* HAL tick comes from 1 ms SysTick, independently of TIM2 updates.
     * Unsigned subtraction also handles the uint32_t tick wraparound.
     */
    uint32_t now_ms = HAL_GetTick();
#if TRAFFIC_LED_TEST
    /* This branch never reads the FSM mode or runs the Ex4 blink logic. */
    if (test_phase >= 4u)
    {
        test_phase = 0u;
        blink_tick_ms = now_ms;
        set_traffic_leds(test_pins[test_phase]);
        return;
    }
    uint32_t steps = (uint32_t)(now_ms - blink_tick_ms) /
                     TRAFFIC_LED_TEST_STEP_MS;
    if (steps != 0u)
    {
        blink_tick_ms += steps * TRAFFIC_LED_TEST_STEP_MS;
        test_phase = (test_phase + steps) % 4u;
        set_traffic_leds(test_pins[test_phase]);
    }
#else
    InputMode mode = input_mode;
    if (mode != blink_mode)
    {
        select_blink_mode(mode, now_ms);
        return;
    }
    uint32_t elapsed_ms = now_ms - blink_tick_ms;
    uint32_t half_periods = elapsed_ms / TRAFFIC_LIGHT_BLINK_HALF_PERIOD_MS;

    if (half_periods != 0u)
    {
        /* Preserve the remainder and phase if foreground service is late. */
        blink_tick_ms += half_periods * TRAFFIC_LIGHT_BLINK_HALF_PERIOD_MS;
        if (((half_periods & 1u) != 0u) && (blink_pins != 0u))
        {
            HAL_GPIO_TogglePin(GPIOA, blink_pins);
        }
    }
#endif
}
