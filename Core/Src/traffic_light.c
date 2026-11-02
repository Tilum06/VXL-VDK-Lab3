#include "traffic_light.h"
#include "main.h"

static uint32_t blink_tick_ms;

void traffic_light_init(void)
{
    /* Equal LOW/HIGH phases give 250 ms on/off for either LED polarity. */
    HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_RESET);
    blink_tick_ms = HAL_GetTick();
}

void traffic_light_process(void)
{
    /* HAL tick comes from 1 ms SysTick, independently of TIM2 updates.
     * Unsigned subtraction also handles the uint32_t tick wraparound.
     */
    uint32_t elapsed_ms = HAL_GetTick() - blink_tick_ms;
    uint32_t half_periods = elapsed_ms / TRAFFIC_LIGHT_BLINK_HALF_PERIOD_MS;

    if (half_periods != 0u)
    {
        /* Preserve the remainder and phase if foreground service is late. */
        blink_tick_ms += half_periods * TRAFFIC_LIGHT_BLINK_HALF_PERIOD_MS;
        if ((half_periods & 1u) != 0u)
        {
            HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
        }
    }
}
