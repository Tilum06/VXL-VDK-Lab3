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

TrafficHandoff fsm_automatic_handoff(void);
/* Schedule the matching phase; complete it with run(0) in the same iteration. */
void fsm_automatic_resume(TrafficHandoff handoff);
/* restart=1 starts fresh at ROAD1_GREEN; restart=0 runs/resumes the current phase. */
void fsm_automatic_run(uint8_t restart);

#endif
