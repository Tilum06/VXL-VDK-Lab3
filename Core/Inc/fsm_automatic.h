#ifndef INC_FSM_AUTOMATIC_H_
#define INC_FSM_AUTOMATIC_H_

#include <stdint.h>

typedef enum {
    ROAD1_GREEN,
    ROAD1_YELLOW,
    ALL_RED_TO_ROAD2,
    ROAD2_GREEN,
    ROAD2_YELLOW,
    ALL_RED_TO_ROAD1,
    TRAFFIC_PHASE_COUNT
} TrafficPhase;

/* Transfer the displayed phase and actual continuous all-red age, not an
 * expired Automatic deadline. Each destination rearms its own phase timer.
 */
typedef struct {
    TrafficPhase phase;
    uint32_t all_red_elapsed_ms;
} TrafficHandoff;

void fsm_automatic_enter(void);
TrafficHandoff fsm_automatic_handoff(void);
void fsm_automatic_resume(TrafficHandoff handoff);
void fsm_automatic_run(void);

#endif
