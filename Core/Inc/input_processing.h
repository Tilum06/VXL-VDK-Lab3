#ifndef INC_INPUT_PROCESSING_H_
#define INC_INPUT_PROCESSING_H_

#include <stdint.h>

typedef enum {
    INPUT_MODE_NORMAL = 1,
    INPUT_MODE_RED,
    INPUT_MODE_YELLOW,
    INPUT_MODE_GREEN
} InputMode;

#define INPUT_DURATION_MIN_S 1u
#define INPUT_DURATION_MAX_S 99u

void input_processing_init(void);
/* Future super-loop entry; never run the application FSM in the TIM2 ISR. */
void fsm_for_input_processing(void);

#endif /* INC_INPUT_PROCESSING_H_ */
