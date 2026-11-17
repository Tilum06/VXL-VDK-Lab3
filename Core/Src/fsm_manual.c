#include "fsm_manual.h"
#include "software_timer.h"
#include "traffic_light.h"
#include "led_7seg.h"

void fsm_manual_enter(void)
{
    switch (traffic_phase) {
    case ROAD1_GREEN: manual_state = MAN_ROAD1_GREEN; break;
    case ROAD1_YELLOW: manual_state = MAN_ROAD1_YELLOW; break;
    case ROAD2_GREEN: manual_state = MAN_ROAD2_GREEN; break;
    case ROAD2_YELLOW: manual_state = MAN_ROAD2_YELLOW; break;
    case ALL_RED_TO_ROAD2: manual_state = MAN_ALL_RED_TO_ROAD2; break;
    case ALL_RED_TO_ROAD1: manual_state = MAN_ALL_RED_TO_ROAD1; break;
    default: manual_state = MAN_ALL_RED_TO_ROAD1; break;
    }
    setTimer(TIMER_MANUAL, 10000);
    led_7seg_blank();
}

void fsm_manual_run(void)
{
    if (input_mode != INPUT_MODE_MANUAL) return;
    uint8_t step = set_pressed;
    led_7seg_blank();
    if (set_pressed || save_pressed) setTimer(TIMER_MANUAL, 10000);
    for (;;) {
        /* A press advances once; the second pass renders the new state now. */
        uint8_t advance = step;
        step = 0;
        switch (manual_state) {
        case MAN_ROAD1_GREEN:
            traffic_light_set(LIGHT_GREEN, LIGHT_RED);
            if (!advance) return;
            manual_state = MAN_ROAD1_YELLOW;
            break;
        case MAN_ROAD1_YELLOW:
            traffic_light_set(LIGHT_YELLOW, LIGHT_RED);
            if (!advance) return;
            manual_state = MAN_ROAD2_GREEN;
            break;
        case MAN_ROAD2_GREEN:
            traffic_light_set(LIGHT_RED, LIGHT_GREEN);
            if (!advance) return;
            manual_state = MAN_ROAD2_YELLOW;
            break;
        case MAN_ROAD2_YELLOW:
            traffic_light_set(LIGHT_RED, LIGHT_YELLOW);
            if (!advance) return;
            manual_state = MAN_ROAD1_GREEN;
            break;
        case MAN_ALL_RED_TO_ROAD2:
            traffic_light_set(LIGHT_RED, LIGHT_RED);
            if (!advance || !isTimerExpired(TIMER_AUTO_PHASE)) return;
            manual_state = MAN_ROAD2_GREEN;
            break;
        case MAN_ALL_RED_TO_ROAD1:
            traffic_light_set(LIGHT_RED, LIGHT_RED);
            if (!advance || !isTimerExpired(TIMER_AUTO_PHASE)) return;
            manual_state = MAN_ROAD1_GREEN;
            break;
        }
    }
}
