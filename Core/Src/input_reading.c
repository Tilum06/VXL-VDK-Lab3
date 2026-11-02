#include "input_reading.h"
#include "main.h"

static GPIO_PinState debounce_buffer1[INPUT_BUTTON_COUNT];
static GPIO_PinState debounce_buffer2[INPUT_BUTTON_COUNT];
static volatile GPIO_PinState button_buffer[INPUT_BUTTON_COUNT];
static uint32_t stable_since_ms[INPUT_BUTTON_COUNT];

static GPIO_TypeDef *const button_ports[INPUT_BUTTON_COUNT] = {
    MODE_GPIO_Port, SET_GPIO_Port, SAVE_GPIO_Port
};
static const uint16_t button_pins[INPUT_BUTTON_COUNT] = {
    MODE_Pin, SET_Pin, SAVE_Pin
};

void input_reading_init(void)
{
    uint32_t now_ms = HAL_GetTick();
    for (uint8_t i = 0u; i < INPUT_BUTTON_COUNT; i++)
    {
        debounce_buffer1[i] = GPIO_PIN_SET;
        debounce_buffer2[i] = GPIO_PIN_SET;
        button_buffer[i] = GPIO_PIN_SET;
        stable_since_ms[i] = now_ms;
    }
}

void button_reading(void)
{
    uint32_t now_ms = HAL_GetTick();
    for (uint8_t i = 0u; i < INPUT_BUTTON_COUNT; i++)
    {
        debounce_buffer2[i] = debounce_buffer1[i];
        debounce_buffer1[i] = HAL_GPIO_ReadPin(button_ports[i], button_pins[i]);

        if (debounce_buffer1[i] != debounce_buffer2[i])
        {
            stable_since_ms[i] = now_ms;
        }
        else if ((uint32_t)(now_ms - stable_since_ms[i]) >=
                 INPUT_DEBOUNCE_STABLE_MS)
        {
            /* Publish only a stable level, for both press and release. */
            button_buffer[i] = debounce_buffer1[i];
        }
    }
}

uint8_t is_button_pressed(uint8_t index)
{
    if (index >= INPUT_BUTTON_COUNT)
    {
        return 0u;
    }
    return button_buffer[index] == GPIO_PIN_RESET;
}

uint8_t is_button_pressed_1s(uint8_t index)
{
    (void)index;
    /* Reserved: no hold/auto-repeat behavior in Ex5. */
    return 0u;
}
