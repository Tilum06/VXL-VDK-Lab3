#ifndef GLOBAL_H
#define GLOBAL_H
#include <stdint.h>

typedef enum {
    INPUT_MODE_NORMAL = 1, INPUT_MODE_RED, INPUT_MODE_YELLOW,
    INPUT_MODE_GREEN, INPUT_MODE_MANUAL
} InputMode;

typedef enum {
    ROAD1_GREEN, ROAD1_YELLOW, ALL_RED_TO_ROAD2,
    ROAD2_GREEN, ROAD2_YELLOW, ALL_RED_TO_ROAD1, TRAFFIC_PHASE_COUNT
} TrafficPhase;

typedef enum {
    MAN_ROAD1_GREEN, MAN_ROAD1_YELLOW, MAN_ALL_RED_TO_ROAD2,
    MAN_ROAD2_GREEN, MAN_ROAD2_YELLOW, MAN_ALL_RED_TO_ROAD1
} ManualState;

extern InputMode input_mode;
extern TrafficPhase traffic_phase;
extern ManualState manual_state;
extern uint8_t red_time, yellow_time, green_time;
/* Mode Controller routes these edges to exactly one active FSM per tick. */
extern uint8_t set_pressed, save_pressed;
uint8_t durations_valid(uint8_t red, uint8_t yellow, uint8_t green);
uint32_t all_red_ms(void);
#endif
