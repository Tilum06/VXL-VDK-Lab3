#include "fsm_setting.h"
#include "global.h"
#include "software_timer.h"
#include "traffic_light.h"
#include "led_7seg.h"

static uint8_t edit_time, blink_on;

void fsm_setting_enter(void)
{
    switch (input_mode) {
    case INPUT_MODE_RED: edit_time = red_time; break;
    case INPUT_MODE_YELLOW: edit_time = yellow_time; break;
    case INPUT_MODE_GREEN: edit_time = green_time; break;
    default: return;
    }
    blink_on = 1;
    setTimer(TIMER_BLINK, 250); /* 250 ms ON + 250 ms OFF = 500 ms. */
    setTimer(TIMER_ERROR, 0);
}

/* Shared by the three setting cases; never changes another colour's value. */
static void edit_colour(uint8_t *saved, uint8_t valid)
{
    if (save_pressed) {
        if (valid) *saved = edit_time;
        else setTimer(TIMER_ERROR, 1000);
    }
}

void fsm_setting_run(void)
{
    if (input_mode < INPUT_MODE_RED || input_mode > INPUT_MODE_GREEN) return;
    if (set_pressed) edit_time = edit_time == 99 ? 1 : edit_time + 1;
    if (isTimerExpired(TIMER_BLINK)) {
        blink_on = !blink_on;
        setTimer(TIMER_BLINK, 250);
    }
    switch (input_mode) {
    case INPUT_MODE_RED:
        traffic_light_set(blink_on ? LIGHT_RED : LIGHT_OFF, blink_on ? LIGHT_RED : LIGHT_OFF);
        edit_colour(&red_time, durations_valid(edit_time, yellow_time, green_time));
        led_7seg_show(2, isTimerExpired(TIMER_ERROR) ? edit_time : 0);
        break;
    case INPUT_MODE_YELLOW:
        traffic_light_set(blink_on ? LIGHT_YELLOW : LIGHT_OFF, blink_on ? LIGHT_YELLOW : LIGHT_OFF);
        edit_colour(&yellow_time, durations_valid(red_time, edit_time, green_time));
        led_7seg_show(3, isTimerExpired(TIMER_ERROR) ? edit_time : 0);
        break;
    case INPUT_MODE_GREEN:
        traffic_light_set(blink_on ? LIGHT_GREEN : LIGHT_OFF, blink_on ? LIGHT_GREEN : LIGHT_OFF);
        edit_colour(&green_time, durations_valid(red_time, yellow_time, edit_time));
        led_7seg_show(4, isTimerExpired(TIMER_ERROR) ? edit_time : 0);
        break;
    default: break;
    }
}
