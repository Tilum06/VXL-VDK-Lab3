#include "fsm_automatic.h"
#include "software_timer.h"
#include "traffic_light.h"
#include "led_7seg.h"

static uint8_t seconds(uint32_t ms) { return (uint8_t)((ms + 999u) / 1000u); }

void fsm_automatic_start(void)
{
    traffic_phase = ROAD1_GREEN;
    setTimer(TIMER_AUTO_PHASE, green_time * 1000u);
}

void fsm_automatic_resume(void)
{
    /* Coloured phases restart with their normal duration. For ALL_RED only,
     * keep the clearance timer inherited on entry to Manual (it kept ticking).
     */
    switch (manual_state) {
    case MAN_ROAD1_GREEN:
        traffic_phase = ROAD1_GREEN;
        setTimer(TIMER_AUTO_PHASE, green_time * 1000u);
        break;
    case MAN_ROAD1_YELLOW:
        traffic_phase = ROAD1_YELLOW;
        setTimer(TIMER_AUTO_PHASE, yellow_time * 1000u);
        break;
    case MAN_ROAD2_GREEN:
        traffic_phase = ROAD2_GREEN;
        setTimer(TIMER_AUTO_PHASE, green_time * 1000u);
        break;
    case MAN_ROAD2_YELLOW:
        traffic_phase = ROAD2_YELLOW;
        setTimer(TIMER_AUTO_PHASE, yellow_time * 1000u);
        break;
    case MAN_ALL_RED_TO_ROAD2: traffic_phase = ALL_RED_TO_ROAD2; break;
    case MAN_ALL_RED_TO_ROAD1: traffic_phase = ALL_RED_TO_ROAD1; break;
    }
}

void fsm_automatic_run(void)
{
    if (input_mode != INPUT_MODE_NORMAL) return;
    /* Render the destination in this same invocation, so transitions never
     * add an extra 10 ms to a phase. Zero clearance is bypassed explicitly.
     */
    for (;;) {
        uint32_t left = timerRemaining(TIMER_AUTO_PHASE);
        uint32_t clearance = all_red_ms();
        switch (traffic_phase) {
        case ROAD1_GREEN:
            traffic_light_set(LIGHT_GREEN, LIGHT_RED);
            led_7seg_show(seconds(left), seconds(left + yellow_time * 1000u + clearance));
            if (!isTimerExpired(TIMER_AUTO_PHASE)) return;
            traffic_phase = ROAD1_YELLOW;
            setTimer(TIMER_AUTO_PHASE, yellow_time * 1000u);
            break;
        case ROAD1_YELLOW:
            traffic_light_set(LIGHT_YELLOW, LIGHT_RED);
            led_7seg_show(seconds(left), seconds(left + clearance));
            if (!isTimerExpired(TIMER_AUTO_PHASE)) return;
            traffic_phase = clearance == 0 ? ROAD2_GREEN : ALL_RED_TO_ROAD2;
            setTimer(TIMER_AUTO_PHASE, clearance == 0 ? green_time * 1000u : clearance);
            break;
        case ALL_RED_TO_ROAD2:
            traffic_light_set(LIGHT_RED, LIGHT_RED);
            led_7seg_show(seconds(left + green_time * 1000u + yellow_time * 1000u + clearance),
                          seconds(left));
            if (!isTimerExpired(TIMER_AUTO_PHASE)) return;
            traffic_phase = ROAD2_GREEN;
            setTimer(TIMER_AUTO_PHASE, green_time * 1000u);
            break;
        case ROAD2_GREEN:
            traffic_light_set(LIGHT_RED, LIGHT_GREEN);
            led_7seg_show(seconds(left + yellow_time * 1000u + clearance), seconds(left));
            if (!isTimerExpired(TIMER_AUTO_PHASE)) return;
            traffic_phase = ROAD2_YELLOW;
            setTimer(TIMER_AUTO_PHASE, yellow_time * 1000u);
            break;
        case ROAD2_YELLOW:
            traffic_light_set(LIGHT_RED, LIGHT_YELLOW);
            led_7seg_show(seconds(left + clearance), seconds(left));
            if (!isTimerExpired(TIMER_AUTO_PHASE)) return;
            traffic_phase = clearance == 0 ? ROAD1_GREEN : ALL_RED_TO_ROAD1;
            setTimer(TIMER_AUTO_PHASE, clearance == 0 ? green_time * 1000u : clearance);
            break;
        case ALL_RED_TO_ROAD1:
            traffic_light_set(LIGHT_RED, LIGHT_RED);
            led_7seg_show(seconds(left),
                          seconds(left + green_time * 1000u + yellow_time * 1000u + clearance));
            if (!isTimerExpired(TIMER_AUTO_PHASE)) return;
            traffic_phase = ROAD1_GREEN;
            setTimer(TIMER_AUTO_PHASE, green_time * 1000u);
            break;
        default:
            traffic_light_set(LIGHT_RED, LIGHT_RED);
            fsm_automatic_start();
            break;
        }
    }
}
