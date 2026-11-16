#include "button.h"
#include "main.h"

typedef struct {
    GPIO_PinState KeyReg0;
    GPIO_PinState KeyReg1;
    GPIO_PinState KeyReg2;
    GPIO_PinState KeyReg3;
} ButtonFilter;

/* Filters and event flags are owned by foreground scheduler tasks. */
static ButtonFilter keys[BUTTON_COUNT];
static volatile ButtonEvents pending_events;
static GPIO_TypeDef *const button_ports[BUTTON_COUNT] = {
    MODE_GPIO_Port, SET_GPIO_Port, SAVE_GPIO_Port
};
static const uint16_t button_pins[BUTTON_COUNT] = {
    MODE_Pin, SET_Pin, SAVE_Pin
};

void button_init(void)
{
    for (uint8_t i = 0u; i < BUTTON_COUNT; i++)
    {
        keys[i].KeyReg0 = GPIO_PIN_SET;
        keys[i].KeyReg1 = GPIO_PIN_SET;
        keys[i].KeyReg2 = GPIO_PIN_SET;
        keys[i].KeyReg3 = GPIO_PIN_SET;
    }
    pending_events = 0u;
}

void button_reading(void)
{
    ButtonEvents new_events = 0u;
    for (uint8_t i = 0u; i < BUTTON_COUNT; i++)
    {
        ButtonFilter *key = &keys[i];
        key->KeyReg0 = key->KeyReg1;
        key->KeyReg1 = key->KeyReg2;
        key->KeyReg2 = HAL_GPIO_ReadPin(button_ports[i], button_pins[i]);

        if ((key->KeyReg0 == key->KeyReg1) &&
            (key->KeyReg1 == key->KeyReg2) &&
            (key->KeyReg2 != key->KeyReg3))
        {
            key->KeyReg3 = key->KeyReg2;
            if (key->KeyReg3 == GPIO_PIN_RESET)
                new_events |= (ButtonEvents)(1u << i);
        }
    }
    /* Publish together after sampling all three buttons. */
    if (new_events != 0u) pending_events |= new_events;
}

static ButtonEvents take_events(ButtonEvents mask)
{
    /* Read+clear is a transaction. Preserve an already-disabled IRQ state;
     * the critical section contains only the flag load/mask/store.
     */
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    ButtonEvents events = pending_events & mask;
    pending_events &= (ButtonEvents)~mask;
    __set_PRIMASK(primask);
    return events;
}

uint8_t is_button_pressed(uint8_t button_id)
{
    if (button_id >= BUTTON_COUNT) return 0u;
    return take_events((ButtonEvents)(1u << button_id)) != 0u;
}

ButtonEvents button_take_events(void)
{
    return take_events(BUTTON_EVENT_MODE | BUTTON_EVENT_SET | BUTTON_EVENT_SAVE);
}
