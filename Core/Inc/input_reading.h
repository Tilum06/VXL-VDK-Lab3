#ifndef INC_INPUT_READING_H_
#define INC_INPUT_READING_H_

#include <stdint.h>

/* Stable time, measured with HAL tick rather than a fixed IRQ count. */
#define INPUT_DEBOUNCE_STABLE_MS 20u

typedef enum {
    INPUT_BUTTON_MODE = 0, /* PA2 */
    INPUT_BUTTON_SET,      /* PA3: increase */
    INPUT_BUTTON_SAVE,     /* PA4: confirm */
    INPUT_BUTTON_COUNT
} InputButton;

/* Initialize before starting TIM2 interrupts; released levels at startup. */
void input_reading_init(void);
/* TIM2 ISR entry: sample all three buttons independently. */
void button_reading(void);
/* Debounced active-LOW level; invalid index returns 0. */
uint8_t is_button_pressed(uint8_t index);
/* Reserved for a later exercise; Ex5 does not generate hold events. */
uint8_t is_button_pressed_1s(uint8_t index);

#endif /* INC_INPUT_READING_H_ */
