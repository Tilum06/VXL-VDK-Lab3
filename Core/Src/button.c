#include "button.h"
#include "main.h"
static uint8_t history[3], stable[3], events;
static const uint16_t pins[3] = {MODE_Pin, SET_Pin, SAVE_Pin};

void button_init(void)
{
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = MODE_Pin | SET_Pin | SAVE_Pin;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);
    events = 0;
    for (unsigned i = 0; i < 3; ++i) history[i] = stable[i] = 7;
}

void button_run(void)
{
    for (unsigned i = 0; i < 3; ++i) {
        history[i] = ((history[i] << 1) |
                     (HAL_GPIO_ReadPin(GPIOA, pins[i]) == GPIO_PIN_SET)) & 7u;
        if (history[i] == 0 && stable[i] != 0) {
            stable[i] = 0;
            events |= (uint8_t)(1u << i);
        } else if (history[i] == 7) {
            stable[i] = 7;
        }
    }
}

uint8_t button_take_events(void)
{
    uint8_t result = events;
    events = 0;
    return result;
}
