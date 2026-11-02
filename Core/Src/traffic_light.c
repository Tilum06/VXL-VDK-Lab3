#include "traffic_light.h"
#include "main.h"
#include "input_processing.h"

#define ALL_TRAFFIC_PINS (RED_LED_Pin | YELLOW_LED_Pin | GREEN_LED_Pin | \
                          RED_LED2_Pin | YELLOW_LED2_Pin | GREEN_LED2_Pin)

static InputMode blink_mode;
static uint16_t blink_pins;
static uint32_t blink_tick_ms;

static void select_blink_mode(InputMode mode, uint32_t now_ms)
{
    /* User-confirmed wiring: anode to +3.3 V, cathode to GPIO. */
    HAL_GPIO_WritePin(GPIOA, ALL_TRAFFIC_PINS, GPIO_PIN_SET);
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
    if (blink_pins != 0u)
    {
        /* Start each selected mode with a complete 250 ms ON phase. */
        HAL_GPIO_WritePin(GPIOA, blink_pins, GPIO_PIN_RESET);
    }
}

void traffic_light_init(void)
{
    select_blink_mode(INPUT_MODE_NORMAL, HAL_GetTick());
}

void traffic_light_process(void)
{
    /* HAL tick comes from 1 ms SysTick, independently of TIM2 updates.
     * Unsigned subtraction also handles the uint32_t tick wraparound.
     */
    uint32_t now_ms = HAL_GetTick();
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
}
