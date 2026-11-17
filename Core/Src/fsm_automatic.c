#include "fsm_automatic.h"
#include "traffic_light.h"
#include "output_display.h"
#include "software_timer.h"

#define AUTOMATIC_REENTRY_GUARD_MS 1000u

static uint32_t phase_duration_ms[TRAFFIC_PHASE_COUNT];
static uint32_t cycle_ms;
static TrafficPhase traffic_phase;
static uint8_t last_display1, last_display2;
static uint8_t start_pending;
static uint8_t reentry_guard;
static TrafficPhase guard_target;

TrafficHandoff fsm_automatic_handoff(void)
{
    TrafficHandoff handoff = {traffic_phase, 0u};
    if ((traffic_phase == ALL_RED_TO_ROAD1) ||
        (traffic_phase == ALL_RED_TO_ROAD2))
        handoff.all_red_elapsed_ms = getTimerElapsed(TIMER_AUTO_ALL_RED);
    return handoff;
}

void fsm_automatic_resume(TrafficHandoff handoff)
{
    traffic_phase = handoff.phase;
    start_pending = 1u;
    reentry_guard = 0u;
    if ((traffic_phase == ALL_RED_TO_ROAD1) ||
        (traffic_phase == ALL_RED_TO_ROAD2))
    {
        guard_target = (traffic_phase == ALL_RED_TO_ROAD1) ? ROAD1_GREEN : ROAD2_GREEN;
        reentry_guard = 1u;
        setTimerElapsed(TIMER_AUTO_ALL_RED, AUTOMATIC_REENTRY_GUARD_MS,
                        handoff.all_red_elapsed_ms);
        /* Preserve only the unfinished part of an existing all-red guard. */
        if (!isTimerExpired(TIMER_AUTO_ALL_RED)) output_display_set_pairs(1u, 1u);
    }
    /* run(0) completes the handoff in this same foreground iteration. */
}

void fsm_automatic_run(uint8_t restart)
{
    uint8_t apply_output = 0u;
    if (restart || cycle_ms == 0u)
    {
        traffic_phase = ROAD1_GREEN;
        reentry_guard = 0u;
        start_pending = 1u;
        apply_output = 1u;
    }
    if (reentry_guard)
    {
        if (!isTimerExpired(TIMER_AUTO_ALL_RED)) return;
        reentry_guard = 0u;
        traffic_phase = guard_target;
        start_pending = 1u;
        apply_output = 1u;
    }
    if (start_pending)
    {
        /* Snapshot saved times once on Normal entry or Manual handoff.
         * Colored handoffs keep the existing GPIO and restart a full phase.
         */
        uint32_t green_ms = (uint32_t)traffic_light_get_green_duration_s() * 1000u;
        uint32_t yellow_ms = (uint32_t)traffic_light_get_yellow_duration_s() * 1000u;
        uint32_t extra_ms = (uint32_t)traffic_light_get_red_duration_s() * 1000u -
                            green_ms - yellow_ms;
        phase_duration_ms[ROAD1_GREEN] = phase_duration_ms[ROAD2_GREEN] = green_ms;
        phase_duration_ms[ROAD1_YELLOW] = phase_duration_ms[ROAD2_YELLOW] = yellow_ms;
        phase_duration_ms[ALL_RED_TO_ROAD2] = extra_ms / 2u;
        phase_duration_ms[ALL_RED_TO_ROAD1] = extra_ms - extra_ms / 2u;
        cycle_ms = green_ms * 2u + yellow_ms * 2u + extra_ms;
        setTimer(TIMER_AUTO_PHASE, phase_duration_ms[traffic_phase]);
        stopTimer(TIMER_AUTO_ALL_RED);
        last_display1 = last_display2 = 0xffu;
        start_pending = 0u;
    }

    /* Skip complete cycles and expired phases, retaining the time remainder.
     * Zero-duration ALL_RED phases are skipped without writing stale colors.
     */
    uint32_t elapsed_ms = getTimerElapsed(TIMER_AUTO_PHASE) % cycle_ms;
    uint8_t phase_expired = isTimerExpired(TIMER_AUTO_PHASE);
    TrafficPhase previous_phase = traffic_phase;
    while (elapsed_ms >= phase_duration_ms[traffic_phase])
    {
        elapsed_ms -= phase_duration_ms[traffic_phase];
        switch (traffic_phase)
        {
        case ROAD1_GREEN:
            traffic_phase = ROAD1_YELLOW;
            break;
        case ROAD1_YELLOW:
            traffic_phase = ALL_RED_TO_ROAD2;
            break;
        case ALL_RED_TO_ROAD2:
            traffic_phase = ROAD2_GREEN;
            break;
        case ROAD2_GREEN:
            traffic_phase = ROAD2_YELLOW;
            break;
        case ROAD2_YELLOW:
            traffic_phase = ALL_RED_TO_ROAD1;
            break;
        case ALL_RED_TO_ROAD1:
        default:
            traffic_phase = ROAD1_GREEN;
            break;
        }
    }
    if (phase_expired)
        setTimerElapsed(TIMER_AUTO_PHASE, phase_duration_ms[traffic_phase], elapsed_ms);
    if (traffic_phase != previous_phase)
    {
        apply_output = 1u;
        if ((traffic_phase == ALL_RED_TO_ROAD1) ||
            (traffic_phase == ALL_RED_TO_ROAD2))
            setTimer(TIMER_AUTO_ALL_RED, AUTOMATIC_REENTRY_GUARD_MS);
        if ((traffic_phase != ALL_RED_TO_ROAD1) &&
            (traffic_phase != ALL_RED_TO_ROAD2)) stopTimer(TIMER_AUTO_ALL_RED);
    }

    /* Each case describes both the displayed colors and the two countdowns. */
    TrafficColor road1_color = TRAFFIC_COLOR_RED;
    TrafficColor road2_color = TRAFFIC_COLOR_RED;
    uint32_t road1 = getTimerRemaining(TIMER_AUTO_PHASE);
    uint32_t road2 = road1;
    switch (traffic_phase)
    {
    case ROAD1_GREEN:
        road1_color = TRAFFIC_COLOR_GREEN;
        road2 += phase_duration_ms[ROAD1_YELLOW] + phase_duration_ms[ALL_RED_TO_ROAD2];
        break;
    case ROAD1_YELLOW:
        road1_color = TRAFFIC_COLOR_YELLOW;
        road2 += phase_duration_ms[ALL_RED_TO_ROAD2];
        break;
    case ALL_RED_TO_ROAD2:
        road1 += phase_duration_ms[ROAD2_GREEN] + phase_duration_ms[ROAD2_YELLOW] +
                 phase_duration_ms[ALL_RED_TO_ROAD1];
        break;
    case ROAD2_GREEN:
        road2_color = TRAFFIC_COLOR_GREEN;
        road1 += phase_duration_ms[ROAD2_YELLOW] + phase_duration_ms[ALL_RED_TO_ROAD1];
        break;
    case ROAD2_YELLOW:
        road2_color = TRAFFIC_COLOR_YELLOW;
        road1 += phase_duration_ms[ALL_RED_TO_ROAD1];
        break;
    case ALL_RED_TO_ROAD1:
        road2 += phase_duration_ms[ROAD1_GREEN] + phase_duration_ms[ROAD1_YELLOW] +
                 phase_duration_ms[ALL_RED_TO_ROAD2];
        break;
    default:
        break;
    }
    if (apply_output) traffic_light_set_roads(road1_color, road2_color);
    /* Round up so 01 remains visible until the actual phase ends. */
    uint8_t first = (uint8_t)((road1 + 999u) / 1000u);
    uint8_t second = (uint8_t)((road2 + 999u) / 1000u);
    if ((first != last_display1) || (second != last_display2))
    {
        output_display_set_pairs(first, second);
        last_display1 = first;
        last_display2 = second;
    }
}
