#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

#include <stdint.h>

typedef enum {
    INPUT_MODE_NORMAL = 1,
    INPUT_MODE_RED,
    INPUT_MODE_YELLOW,
    INPUT_MODE_GREEN,
    INPUT_MODE_MANUAL
} InputMode;

#define INPUT_DURATION_MIN_S 1u
#define INPUT_DURATION_MAX_S 99u

/* Foreground-owned mode, visible in debugger Watch/Live Expressions. */
extern volatile InputMode input_mode;

#endif /* INC_GLOBAL_H_ */
