#ifndef INC_FSM_MANUAL_H_
#define INC_FSM_MANUAL_H_

#include <stdint.h>
#include "fsm_automatic.h"

#define MANUAL_TIMEOUT_MS 10000u
#define MANUAL_ALL_RED_GUARD_MS 1000u

typedef enum {
    MAN_ROAD1_GREEN,
    MAN_ROAD1_YELLOW,
    /* Entry-only: preserve Automatic ALL_RED and its pending road. */
    MAN_ALL_RED_TO_ROAD2,
    MAN_ROAD2_GREEN,
    MAN_ROAD2_YELLOW,
    MAN_ALL_RED_TO_ROAD1
} ManualState;

void fsm_manual_enter(TrafficHandoff handoff);
TrafficHandoff fsm_manual_handoff(void);
uint8_t fsm_manual_timed_out(void);
void fsm_manual_run(uint8_t set_edge);

#endif
