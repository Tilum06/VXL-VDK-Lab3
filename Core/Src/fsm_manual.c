#include "fsm_manual.h"
#include "traffic_light.h"
#include "software_timer.h"

static ManualState manual_state;

static void apply_state(void)
{
    switch (manual_state)
    {
    case MAN_ROAD1_GREEN:
        traffic_light_set_roads(TRAFFIC_COLOR_GREEN, TRAFFIC_COLOR_RED);
        break;
    case MAN_ROAD1_YELLOW:
        traffic_light_set_roads(TRAFFIC_COLOR_YELLOW, TRAFFIC_COLOR_RED);
        break;
    case MAN_ROAD2_GREEN:
        traffic_light_set_roads(TRAFFIC_COLOR_RED, TRAFFIC_COLOR_GREEN);
        break;
    case MAN_ROAD2_YELLOW:
        traffic_light_set_roads(TRAFFIC_COLOR_RED, TRAFFIC_COLOR_YELLOW);
        break;
    case MAN_ALL_RED_TO_ROAD2:
    case MAN_ALL_RED_TO_ROAD1:
    default:
        traffic_light_set_roads(TRAFFIC_COLOR_RED, TRAFFIC_COLOR_RED);
        break;
    }
}

void fsm_manual_enter(TrafficHandoff handoff)
{
    switch (handoff.phase)
    {
    case ROAD1_GREEN:
        manual_state = MAN_ROAD1_GREEN;
        break;
    case ROAD1_YELLOW:
        manual_state = MAN_ROAD1_YELLOW;
        break;
    case ROAD2_GREEN:
        manual_state = MAN_ROAD2_GREEN;
        break;
    case ROAD2_YELLOW:
        manual_state = MAN_ROAD2_YELLOW;
        break;
    case ALL_RED_TO_ROAD2:
        manual_state = MAN_ALL_RED_TO_ROAD2;
        break;
    case ALL_RED_TO_ROAD1:
    default:
        manual_state = MAN_ALL_RED_TO_ROAD1;
        break;
    }
    uint32_t age = handoff.all_red_elapsed_ms;
    if (age > MANUAL_ALL_RED_GUARD_MS) age = MANUAL_ALL_RED_GUARD_MS;
    stopTimer(TIMER_MANUAL_ENTRY);
    if ((manual_state == MAN_ALL_RED_TO_ROAD1) ||
        (manual_state == MAN_ALL_RED_TO_ROAD2))
        setTimerElapsed(TIMER_MANUAL_ENTRY, MANUAL_ALL_RED_GUARD_MS, age);
    setTimer(TIMER_MANUAL_TIMEOUT, MANUAL_TIMEOUT_MS);
    /* The matching colors are already on. Transfer ownership without a write. */
}

TrafficHandoff fsm_manual_handoff(void)
{
    TrafficHandoff handoff = {ALL_RED_TO_ROAD1, 0u};
    switch (manual_state)
    {
    case MAN_ROAD1_GREEN:
        handoff.phase = ROAD1_GREEN;
        break;
    case MAN_ROAD1_YELLOW:
        handoff.phase = ROAD1_YELLOW;
        break;
    case MAN_ROAD2_GREEN:
        handoff.phase = ROAD2_GREEN;
        break;
    case MAN_ROAD2_YELLOW:
        handoff.phase = ROAD2_YELLOW;
        break;
    case MAN_ALL_RED_TO_ROAD2:
        handoff.phase = ALL_RED_TO_ROAD2;
        handoff.all_red_elapsed_ms = getTimerElapsed(TIMER_MANUAL_ENTRY);
        break;
    case MAN_ALL_RED_TO_ROAD1:
        handoff.all_red_elapsed_ms = getTimerElapsed(TIMER_MANUAL_ENTRY);
        break;
    default:
        break;
    }
    return handoff;
}

uint8_t fsm_manual_timed_out(void)
{
    return isTimerExpired(TIMER_MANUAL_TIMEOUT);
}

void fsm_manual_run(uint8_t set_edge)
{
    ManualState previous_state = manual_state;
    uint8_t guard_elapsed = isTimerExpired(TIMER_MANUAL_ENTRY);

    /* The four colored states advance directly on one SET edge.
     * ALL_RED cases are entry-only transfers from Automatic and finish by time.
     */
    switch (manual_state)
    {
    case MAN_ROAD1_GREEN:
        if (set_edge) manual_state = MAN_ROAD1_YELLOW;
        break;
    case MAN_ROAD1_YELLOW:
        if (set_edge) manual_state = MAN_ROAD2_GREEN;
        break;
    case MAN_ALL_RED_TO_ROAD2:
        if (guard_elapsed) manual_state = MAN_ROAD2_GREEN;
        break;
    case MAN_ROAD2_GREEN:
        if (set_edge) manual_state = MAN_ROAD2_YELLOW;
        break;
    case MAN_ROAD2_YELLOW:
        if (set_edge) manual_state = MAN_ROAD1_GREEN;
        break;
    case MAN_ALL_RED_TO_ROAD1:
        if (guard_elapsed) manual_state = MAN_ROAD1_GREEN;
        break;
    default:
        /* Recover through all-red, with a fresh guard. */
        manual_state = MAN_ALL_RED_TO_ROAD1;
        break;
    }
    if (manual_state != previous_state)
    {
        stopTimer(TIMER_MANUAL_ENTRY);
        if ((manual_state == MAN_ALL_RED_TO_ROAD1) ||
            (manual_state == MAN_ALL_RED_TO_ROAD2))
            setTimer(TIMER_MANUAL_ENTRY, MANUAL_ALL_RED_GUARD_MS);
        /* Internal guard completion is not a valid SET and cannot renew timeout. */
        if ((previous_state != MAN_ALL_RED_TO_ROAD1) &&
            (previous_state != MAN_ALL_RED_TO_ROAD2))
            setTimer(TIMER_MANUAL_TIMEOUT, MANUAL_TIMEOUT_MS);
        apply_state();
    }
}
