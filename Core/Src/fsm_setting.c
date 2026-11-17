#include "fsm_setting.h"
#include "global.h"
#include "software_timer.h"
#include "traffic_light.h"
#include "led_7seg.h"

static uint8_t edit_time, blink_on;

void fsm_setting_enter(void)
{
    switch (input_mode) {
    case MODE_SET_RED: edit_time = red_time; break;
    case MODE_SET_YELLOW: edit_time = yellow_time; break;
    case MODE_SET_GREEN: edit_time = green_time; break;
    default: return;
    }
    blink_on = 1;
    setTimer(TIMER_BLINK, 250); /* 250 ms ON + 250 ms OFF = 500 ms. */
    setTimer(TIMER_ERROR, 0);
}

void fsm_setting_run(void)
{
    switch (input_mode) {
    case MODE_SET_RED:
        if (isTimerExpired(TIMER_BLINK)) {
            blink_on = !blink_on;
            setTimer(TIMER_BLINK, 250);
        }
        traffic_light_set(blink_on ? LIGHT_RED : LIGHT_OFF,
                          blink_on ? LIGHT_RED : LIGHT_OFF);

        if (set_pressed) {
            edit_time = edit_time == 99 ? 1 : edit_time + 1;
        }
        if (save_pressed && durations_valid(edit_time, yellow_time, green_time)) {
            red_time = edit_time;
        }
        if (save_pressed && !durations_valid(edit_time, yellow_time, green_time)) {
            setTimer(TIMER_ERROR, 1000);
        }
        led_7seg_show(2, isTimerExpired(TIMER_ERROR) ? edit_time : 0);
        break;

    case MODE_SET_YELLOW:
        if (isTimerExpired(TIMER_BLINK)) {
            blink_on = !blink_on;
            setTimer(TIMER_BLINK, 250);
        }
        traffic_light_set(blink_on ? LIGHT_YELLOW : LIGHT_OFF,
                          blink_on ? LIGHT_YELLOW : LIGHT_OFF);

        if (set_pressed) {
            edit_time = edit_time == 99 ? 1 : edit_time + 1;
        }
        if (save_pressed && durations_valid(red_time, edit_time, green_time)) {
            yellow_time = edit_time;
        }
        if (save_pressed && !durations_valid(red_time, edit_time, green_time)) {
            setTimer(TIMER_ERROR, 1000);
        }
        led_7seg_show(3, isTimerExpired(TIMER_ERROR) ? edit_time : 0);
        break;

    case MODE_SET_GREEN:
        if (isTimerExpired(TIMER_BLINK)) {
            blink_on = !blink_on;
            setTimer(TIMER_BLINK, 250);
        }
        traffic_light_set(blink_on ? LIGHT_GREEN : LIGHT_OFF,
                          blink_on ? LIGHT_GREEN : LIGHT_OFF);

        if (set_pressed) {
            edit_time = edit_time == 99 ? 1 : edit_time + 1;
        }
        if (save_pressed && durations_valid(red_time, yellow_time, edit_time)) {
            green_time = edit_time;
        }
        if (save_pressed && !durations_valid(red_time, yellow_time, edit_time)) {
            setTimer(TIMER_ERROR, 1000);
        }
        led_7seg_show(4, isTimerExpired(TIMER_ERROR) ? edit_time : 0);
        break;

    default:
        break;
    }
}
