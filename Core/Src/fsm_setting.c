#include "fsm_setting.h"
#include "traffic_light.h"
#include "output_display.h"
#include "software_timer.h"

static InputMode setting_mode;
static TrafficColor setting_color;
static uint8_t edit_duration_s;
static uint8_t save_rejected;

static void update_display(void)
{
    output_display_set_pairs((uint8_t)setting_mode,
                             save_rejected ? 0u : edit_duration_s);
}

void fsm_setting_enter(InputMode mode)
{
    setting_mode = mode;
    save_rejected = 0u;
    setTimer(TIMER_SETTING_BLINK, TRAFFIC_LIGHT_BLINK_HALF_PERIOD_MS);
    stopTimer(TIMER_SETTING_ERROR);
    switch (setting_mode)
    {
    case INPUT_MODE_RED:
        setting_color = TRAFFIC_COLOR_RED;
        edit_duration_s = traffic_light_get_red_duration_s();
        break;
    case INPUT_MODE_YELLOW:
        setting_color = TRAFFIC_COLOR_YELLOW;
        edit_duration_s = traffic_light_get_yellow_duration_s();
        break;
    case INPUT_MODE_GREEN:
        setting_color = TRAFFIC_COLOR_GREEN;
        edit_duration_s = traffic_light_get_green_duration_s();
        break;
    default:
        return;
    }
    traffic_light_set_blink_color(setting_color);
    update_display();
}

void fsm_setting_run(uint8_t set_edge, uint8_t save_edge)
{
    /* Independent events: simultaneous SET+SAVE saves the incremented value. */
    if (set_edge)
    {
        save_rejected = 0u;
        stopTimer(TIMER_SETTING_ERROR);
        edit_duration_s = (edit_duration_s >= INPUT_DURATION_MAX_S) ?
                          INPUT_DURATION_MIN_S : edit_duration_s + 1u;
    }
    if (save_edge)
    {
        uint8_t accepted;
        switch (setting_mode)
        {
        case INPUT_MODE_RED:
            accepted = traffic_light_set_red_duration_s(edit_duration_s);
            break;
        case INPUT_MODE_YELLOW:
            accepted = traffic_light_set_yellow_duration_s(edit_duration_s);
            break;
        case INPUT_MODE_GREEN:
            accepted = traffic_light_set_green_duration_s(edit_duration_s);
            break;
        default:
            return;
        }
        save_rejected = !accepted;
        if (save_rejected) setTimer(TIMER_SETTING_ERROR, 1000u);
        if (!save_rejected) stopTimer(TIMER_SETTING_ERROR);
    }
    uint8_t error_expired = save_rejected && isTimerExpired(TIMER_SETTING_ERROR);
    if (error_expired)
    {
        save_rejected = 0u;
        stopTimer(TIMER_SETTING_ERROR);
    }
    if (set_edge || save_edge || error_expired) update_display();

    uint32_t half_periods = consumeTimerPeriods(TIMER_SETTING_BLINK);
    if ((half_periods & 1u) != 0u) traffic_light_toggle_color(setting_color);
}
