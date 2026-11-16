#include "fsm_automatic.h"
#include "traffic_light.h"
#include "output_display.h"
#include "software_timer.h"

#define AUTOMATIC_REENTRY_GUARD_MS 1000u

static uint32_t phase_duration_ms[TRAFFIC_PHASE_COUNT];
static uint32_t cycle_ms;
static TrafficPhase traffic_phase;
static uint8_t last_display1, last_display2;

static uint8_t reentry_guard;
static TrafficPhase guard_target;

static void apply_traffic_phase(void)
{
    switch (traffic_phase)
    {
    case ROAD1_GREEN:
        traffic_light_set_roads(TRAFFIC_COLOR_GREEN, TRAFFIC_COLOR_RED);
        break;
    case ROAD1_YELLOW:
        traffic_light_set_roads(TRAFFIC_COLOR_YELLOW, TRAFFIC_COLOR_RED);
        break;
    case ROAD2_GREEN:
        traffic_light_set_roads(TRAFFIC_COLOR_RED, TRAFFIC_COLOR_GREEN);
        break;
    case ROAD2_YELLOW:
        traffic_light_set_roads(TRAFFIC_COLOR_RED, TRAFFIC_COLOR_YELLOW);
        break;
    case ALL_RED_TO_ROAD2:
    case ALL_RED_TO_ROAD1:
    default:
        traffic_light_set_roads(TRAFFIC_COLOR_RED, TRAFFIC_COLOR_RED);
        break;
    }
}

static void start_normal_traffic(TrafficPhase phase,
                                 uint8_t apply_output)
{
    uint32_t green_duration_s = traffic_light_get_green_duration_s();
    uint32_t yellow_duration_s = traffic_light_get_yellow_duration_s();
    uint32_t red_duration_s = traffic_light_get_red_duration_s();
    uint32_t green_ms = green_duration_s * 1000u;
    uint32_t yellow_ms = (uint32_t)yellow_duration_s * 1000u;
    uint32_t extra_ms = ((uint32_t)red_duration_s - green_duration_s -
                         yellow_duration_s) * 1000u;
    phase_duration_ms[ROAD1_GREEN] = phase_duration_ms[ROAD2_GREEN] = green_ms;
    phase_duration_ms[ROAD1_YELLOW] = phase_duration_ms[ROAD2_YELLOW] = yellow_ms;
    phase_duration_ms[ALL_RED_TO_ROAD2] = extra_ms / 2u;
    phase_duration_ms[ALL_RED_TO_ROAD1] = extra_ms - extra_ms / 2u;
    cycle_ms = green_ms * 2u + yellow_ms * 2u + extra_ms;
    traffic_phase = phase;
    setTimer(TIMER_AUTO_PHASE, phase_duration_ms[traffic_phase]);
    stopTimer(TIMER_AUTO_ALL_RED);
    last_display1 = last_display2 = 0xffu;
    if (apply_output) apply_traffic_phase();
}

static void display_countdown(void)
{
    uint32_t remaining = getTimerRemaining(TIMER_AUTO_PHASE);
    uint32_t road1 = remaining, road2 = remaining;
    switch (traffic_phase)
    {
    case ROAD1_GREEN:
        road2 += phase_duration_ms[ROAD1_YELLOW] + phase_duration_ms[ALL_RED_TO_ROAD2];
        break;
    case ROAD1_YELLOW:
        road2 += phase_duration_ms[ALL_RED_TO_ROAD2];
        break;
    case ALL_RED_TO_ROAD2:
        road1 += phase_duration_ms[ROAD2_GREEN] + phase_duration_ms[ROAD2_YELLOW] +
                 phase_duration_ms[ALL_RED_TO_ROAD1];
        break;
    case ROAD2_GREEN:
        road1 += phase_duration_ms[ROAD2_YELLOW] + phase_duration_ms[ALL_RED_TO_ROAD1];
        break;
    case ROAD2_YELLOW:
        road1 += phase_duration_ms[ALL_RED_TO_ROAD1];
        break;
    case ALL_RED_TO_ROAD1:
        road2 += phase_duration_ms[ROAD1_GREEN] + phase_duration_ms[ROAD1_YELLOW] +
                 phase_duration_ms[ALL_RED_TO_ROAD2];
        break;
    default:
        break;
    }
    /* Ceil: show 01 until the actual color changes, including half seconds. */
    uint8_t first = (uint8_t)((road1 + 999u) / 1000u);
    uint8_t second = (uint8_t)((road2 + 999u) / 1000u);
    if ((first != last_display1) || (second != last_display2))
    {
        output_display_set_pairs(first, second);
        last_display1 = first;
        last_display2 = second;
    }
}

static void process_normal_traffic(void)
{
    uint32_t elapsed_ms = getTimerElapsed(TIMER_AUTO_PHASE);
    uint8_t phase_expired = isTimerExpired(TIMER_AUTO_PHASE);
    /* Skip whole cycles without issuing obsolete GPIO transitions. */
    elapsed_ms %= cycle_ms;
    TrafficPhase previous_phase = traffic_phase;
    while (elapsed_ms >= phase_duration_ms[traffic_phase])
    {
        uint32_t duration = phase_duration_ms[traffic_phase];
        elapsed_ms -= duration;
        /* One explicit transition per iteration; retain catch-up and zero phases. */
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
        apply_traffic_phase();
        /* Guard age follows the GPIO actually applied, including late loops. */
        if ((traffic_phase == ALL_RED_TO_ROAD1) ||
            (traffic_phase == ALL_RED_TO_ROAD2))
            setTimer(TIMER_AUTO_ALL_RED, AUTOMATIC_REENTRY_GUARD_MS);
        if ((traffic_phase != ALL_RED_TO_ROAD1) &&
            (traffic_phase != ALL_RED_TO_ROAD2)) stopTimer(TIMER_AUTO_ALL_RED);
    }
    display_countdown();
}


void fsm_automatic_enter(void)
{
    reentry_guard = 0u;
    start_normal_traffic(ROAD1_GREEN, 1u);
}

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
    reentry_guard = 0u;
    if ((handoff.phase == ALL_RED_TO_ROAD1) ||
        (handoff.phase == ALL_RED_TO_ROAD2))
    {
        guard_target = (handoff.phase == ALL_RED_TO_ROAD1) ? ROAD1_GREEN : ROAD2_GREEN;
        uint32_t age = handoff.all_red_elapsed_ms;
        if (age >= AUTOMATIC_REENTRY_GUARD_MS)
        {
            start_normal_traffic(guard_target, 1u);
            return;
        }
        reentry_guard = 1u;
        traffic_phase = handoff.phase;
        setTimerElapsed(TIMER_AUTO_ALL_RED, AUTOMATIC_REENTRY_GUARD_MS, age);
        /* Both roads already red; retain them while completing the guard. */
        output_display_set_pairs(1u, 1u);
        return;
    }
    /* Same colors: no GPIO rewrite, restart the matching phase's full time. */
    start_normal_traffic(handoff.phase, 0u);
}

void fsm_automatic_run(void)
{
    if (reentry_guard)
    {
        if (!isTimerExpired(TIMER_AUTO_ALL_RED))
            return;
        reentry_guard = 0u;
        /* Start the correct road with a full phase after the remaining guard. */
        start_normal_traffic(guard_target, 1u);
    }
    process_normal_traffic();
}
