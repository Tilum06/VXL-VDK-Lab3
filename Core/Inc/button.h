#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include <stdint.h>

typedef enum {
    BUTTON_MODE = 0, /* PA2, active LOW */
    BUTTON_SET,      /* PA3 */
    BUTTON_SAVE,     /* PA4 */
    BUTTON_COUNT
} ButtonId;

typedef uint8_t ButtonEvents;
#define BUTTON_EVENT_MODE ((ButtonEvents)(1u << BUTTON_MODE))
#define BUTTON_EVENT_SET  ((ButtonEvents)(1u << BUTTON_SET))
#define BUTTON_EVENT_SAVE ((ButtonEvents)(1u << BUTTON_SAVE))

/* Call before starting TIM2; all keys begin in the released state. */
void button_init(void);
/* TIM2 ISR: one sample per callback, normally every 10 ms. No hold repeat. */
void button_reading(void);
/* Consume one pending press, clearing only that key's event. Not a held level. */
uint8_t is_button_pressed(uint8_t button_id);
/* Atomically consume all pending presses once per foreground iteration.
 * Each key is a flag: multiple presses before consumption coalesce to one.
 */
ButtonEvents button_take_events(void);

#endif
