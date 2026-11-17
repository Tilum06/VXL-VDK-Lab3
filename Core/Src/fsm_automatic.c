#include "fsm_automatic.h"
#include "traffic_light.h"
#include "output_display.h"
#include "software_timer.h"

#define ALL_RED_GUARD_MS 1000u

static TrafficPhase traffic_phase;
static uint32_t phase_time[TRAFFIC_PHASE_COUNT];
static uint32_t cycle_time;
static uint8_t need_start;
static uint8_t last_display1 = 0xffu;
static uint8_t last_display2 = 0xffu;

/* Save the current phase when switching from Automatic to Manual. */
TrafficHandoff fsm_automatic_handoff(void)
{
    TrafficHandoff handoff = {traffic_phase, 0u};

    if (traffic_phase == ALL_RED_TO_ROAD1 ||
        traffic_phase == ALL_RED_TO_ROAD2)
    {
        handoff.all_red_elapsed_ms = getTimerElapsed(TIMER_AUTO_ALL_RED);
    }
    return handoff;
}

/* Restore the matching phase after leaving Manual. */
void fsm_automatic_resume(TrafficHandoff handoff)
{
    traffic_phase = handoff.phase;
    need_start = 1u;

    if (traffic_phase == ALL_RED_TO_ROAD1 ||
        traffic_phase == ALL_RED_TO_ROAD2)
    {
        /* Only an existing ALL_RED handoff requires a remaining guard. */
        setTimerElapsed(TIMER_AUTO_ALL_RED, ALL_RED_GUARD_MS,
                        handoff.all_red_elapsed_ms);
        if (!isTimerExpired(TIMER_AUTO_ALL_RED))
        {
            output_display_set_pairs(1u, 1u);
        }
    }
}

void fsm_automatic_run(uint8_t restart)
{
    uint8_t update_led = 0u;

    /* Start normally at ROAD1_GREEN, or resume a phase from Manual. */
    if (restart || cycle_time == 0u)
    {
        traffic_phase = ROAD1_GREEN;
        need_start = 1u;
        update_led = 1u;
        stopTimer(TIMER_AUTO_ALL_RED);
    }

    if (need_start)
    {
        if (traffic_phase == ALL_RED_TO_ROAD1 ||
            traffic_phase == ALL_RED_TO_ROAD2)
        {
            /* Do not introduce a guard for ordinary colored phases. */
            if (!isTimerExpired(TIMER_AUTO_ALL_RED)) return;

            traffic_phase = (traffic_phase == ALL_RED_TO_ROAD1) ?
                            ROAD1_GREEN : ROAD2_GREEN;
            update_led = 1u;
        }

        uint32_t green = (uint32_t)traffic_light_get_green_duration_s() * 1000u;
        uint32_t yellow = (uint32_t)traffic_light_get_yellow_duration_s() * 1000u;
        uint32_t red = (uint32_t)traffic_light_get_red_duration_s() * 1000u;
        uint32_t extra_red = red - green - yellow;

        phase_time[ROAD1_GREEN] = phase_time[ROAD2_GREEN] = green;
        phase_time[ROAD1_YELLOW] = phase_time[ROAD2_YELLOW] = yellow;
        phase_time[ALL_RED_TO_ROAD2] = extra_red / 2u;
        phase_time[ALL_RED_TO_ROAD1] = extra_red - extra_red / 2u;
        cycle_time = green * 2u + yellow * 2u + extra_red;

        setTimer(TIMER_AUTO_PHASE, phase_time[traffic_phase]);
        stopTimer(TIMER_AUTO_ALL_RED);
        last_display1 = last_display2 = 0xffu;
        need_start = 0u;
    }

    /* Preserve the time remainder when the main loop is delayed. */
    uint32_t elapsed = getTimerElapsed(TIMER_AUTO_PHASE) % cycle_time;
    uint8_t timer_expired = isTimerExpired(TIMER_AUTO_PHASE);
    TrafficPhase old_phase = traffic_phase;

    TrafficColor road1_color = TRAFFIC_COLOR_RED;
    TrafficColor road2_color = TRAFFIC_COLOR_RED;
    uint32_t road1_time = 0u;
    uint32_t road2_time = 0u;
    uint32_t remaining = 0u;

    /* Each case: check timer -> select next phase -> prepare LEDs/countdown. */
    while (1)
    {
        switch (traffic_phase)
        {
        case ROAD1_GREEN:
            if (elapsed >= phase_time[ROAD1_GREEN])
            {
                elapsed -= phase_time[ROAD1_GREEN];
                traffic_phase = ROAD1_YELLOW;
                continue;
            }
            remaining = phase_time[ROAD1_GREEN] - elapsed;
            road1_color = TRAFFIC_COLOR_GREEN;
            road1_time = remaining;
            road2_time = remaining + phase_time[ROAD1_YELLOW] +
                         phase_time[ALL_RED_TO_ROAD2];
            break;

        case ROAD1_YELLOW:
            if (elapsed >= phase_time[ROAD1_YELLOW])
            {
                elapsed -= phase_time[ROAD1_YELLOW];
                traffic_phase = ALL_RED_TO_ROAD2;
                continue;
            }
            remaining = phase_time[ROAD1_YELLOW] - elapsed;
            road1_color = TRAFFIC_COLOR_YELLOW;
            road1_time = remaining;
            road2_time = remaining + phase_time[ALL_RED_TO_ROAD2];
            break;

        case ALL_RED_TO_ROAD2:
            if (elapsed >= phase_time[ALL_RED_TO_ROAD2])
            {
                elapsed -= phase_time[ALL_RED_TO_ROAD2];
                traffic_phase = ROAD2_GREEN;
                continue;
            }
            remaining = phase_time[ALL_RED_TO_ROAD2] - elapsed;
            road1_time = remaining + phase_time[ROAD2_GREEN] +
                         phase_time[ROAD2_YELLOW] + phase_time[ALL_RED_TO_ROAD1];
            road2_time = remaining;
            break;

        case ROAD2_GREEN:
            if (elapsed >= phase_time[ROAD2_GREEN])
            {
                elapsed -= phase_time[ROAD2_GREEN];
                traffic_phase = ROAD2_YELLOW;
                continue;
            }
            remaining = phase_time[ROAD2_GREEN] - elapsed;
            road2_color = TRAFFIC_COLOR_GREEN;
            road2_time = remaining;
            road1_time = remaining + phase_time[ROAD2_YELLOW] +
                         phase_time[ALL_RED_TO_ROAD1];
            break;

        case ROAD2_YELLOW:
            if (elapsed >= phase_time[ROAD2_YELLOW])
            {
                elapsed -= phase_time[ROAD2_YELLOW];
                traffic_phase = ALL_RED_TO_ROAD1;
                continue;
            }
            remaining = phase_time[ROAD2_YELLOW] - elapsed;
            road2_color = TRAFFIC_COLOR_YELLOW;
            road2_time = remaining;
            road1_time = remaining + phase_time[ALL_RED_TO_ROAD1];
            break;

        case ALL_RED_TO_ROAD1:
            if (elapsed >= phase_time[ALL_RED_TO_ROAD1])
            {
                elapsed -= phase_time[ALL_RED_TO_ROAD1];
                traffic_phase = ROAD1_GREEN;
                continue;
            }
            remaining = phase_time[ALL_RED_TO_ROAD1] - elapsed;
            road1_time = remaining;
            road2_time = remaining + phase_time[ROAD1_GREEN] +
                         phase_time[ROAD1_YELLOW] + phase_time[ALL_RED_TO_ROAD2];
            break;

        default:
            traffic_phase = ROAD1_GREEN;
            elapsed = 0u;
            update_led = 1u;
            continue;
        }
        break;
    }

    if (timer_expired)
    {
        setTimerElapsed(TIMER_AUTO_PHASE, phase_time[traffic_phase], elapsed);
    }

    if (traffic_phase != old_phase)
    {
        update_led = 1u;
        if (traffic_phase == ALL_RED_TO_ROAD1 ||
            traffic_phase == ALL_RED_TO_ROAD2)
        {
            setTimer(TIMER_AUTO_ALL_RED, ALL_RED_GUARD_MS);
        }
        else
        {
            stopTimer(TIMER_AUTO_ALL_RED);
        }
    }

    if (update_led)
    {
        traffic_light_set_roads(road1_color, road2_color);
    }

    uint8_t display1 = (uint8_t)((road1_time + 999u) / 1000u);
    uint8_t display2 = (uint8_t)((road2_time + 999u) / 1000u);

    if (display1 != last_display1 || display2 != last_display2)
    {
        output_display_set_pairs(display1, display2);
        last_display1 = display1;
        last_display2 = display2;
    }
}
