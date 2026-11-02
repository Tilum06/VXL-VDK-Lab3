#ifndef INC_INPUT_READING_H_
#define INC_INPUT_READING_H_

#include <stdint.h>

/* Planned TIM2 sampling period; hardware timer is not configured here. */
#define INPUT_READING_TIMER_PERIOD_MS 10u

typedef enum {
    INPUT_BUTTON_MODE = 0, /* PA2 */
    INPUT_BUTTON_SET,      /* PA3: increase */
    INPUT_BUTTON_SAVE,     /* PA4: confirm */
    INPUT_BUTTON_COUNT
} InputButton;

/* Skeleton only: no GPIO reads or events until Exercise 5. */
void input_reading_init(void);
/* Future TIM2 ISR entry: bounded sampling, no HAL_Delay. */
void button_reading(void);
/* Level queries; edge detection belongs to input_processing. */
uint8_t is_button_pressed(uint8_t index);
uint8_t is_button_pressed_1s(uint8_t index);

#endif /* INC_INPUT_READING_H_ */
