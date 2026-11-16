#include "input_processing.h"
#include "button.h"
#include "fsm_automatic.h"
#include "fsm_setting.h"
#include "fsm_manual.h"
#include "software_timer.h"
#include "output_display.h"

volatile InputMode input_mode = INPUT_MODE_NORMAL;
static InputMode active_mode;

void input_processing_init(void)
{
    input_mode = INPUT_MODE_NORMAL;
    active_mode = (InputMode)0;
}

void fsm_for_input_processing(void)
{
    timerRun();
    /* One atomic snapshot consumes even ignored events. FSMs receive values,
     * never read/clear the shared button flags themselves.
     */
    ButtonEvents events = button_take_events();
    uint8_t mode_edge = (events & BUTTON_EVENT_MODE) != 0u;
    uint8_t set_edge = (events & BUTTON_EVENT_SET) != 0u;
    uint8_t save_edge = (events & BUTTON_EVENT_SAVE) != 0u;

    /* MODE wins. At the exact timeout boundary, timeout wins over SET.
     * Guard independent ifs to allow at most one mode transition per call.
     */
    switch (input_mode)
    {
    case INPUT_MODE_NORMAL:
        if (mode_edge) input_mode = INPUT_MODE_RED;
        if (!mode_edge && save_edge) input_mode = INPUT_MODE_MANUAL;
        break;
    case INPUT_MODE_RED:
        if (mode_edge) input_mode = INPUT_MODE_YELLOW;
        break;
    case INPUT_MODE_YELLOW:
        if (mode_edge) input_mode = INPUT_MODE_GREEN;
        break;
    case INPUT_MODE_GREEN:
        if (mode_edge) input_mode = INPUT_MODE_NORMAL;
        break;
    case INPUT_MODE_MANUAL:
        if (mode_edge) input_mode = INPUT_MODE_NORMAL;
        if (!mode_edge && fsm_manual_timed_out()) input_mode = INPUT_MODE_NORMAL;
        break;
    default:
        input_mode = INPUT_MODE_NORMAL;
        break;
    }

    if (input_mode != active_mode)
    {
        InputMode previous_mode = active_mode;
        TrafficHandoff handoff = {ROAD1_GREEN, 0u};
        /* Capture all-red age before disabling the departing mode's timers. */
        if (previous_mode == INPUT_MODE_NORMAL) handoff = fsm_automatic_handoff();
        if (previous_mode == INPUT_MODE_MANUAL) handoff = fsm_manual_handoff();
        stopAllTimers();
        active_mode = input_mode;
        /* Mode changes discard simultaneous SET/SAVE and unsaved edits. */
        set_edge = save_edge = 0u;
        switch (active_mode)
        {
        case INPUT_MODE_NORMAL:
            if (previous_mode == INPUT_MODE_MANUAL)
                fsm_automatic_resume(handoff);
            if (previous_mode != INPUT_MODE_MANUAL) fsm_automatic_enter();
            break;
        case INPUT_MODE_RED:
        case INPUT_MODE_YELLOW:
        case INPUT_MODE_GREEN:
            fsm_setting_enter(active_mode);
            break;
        case INPUT_MODE_MANUAL:
            fsm_manual_enter(handoff);
            break;
        default:
            break;
        }
        output_display_set_enabled(active_mode != INPUT_MODE_MANUAL);
    }

    /* Exclusive ownership: inactive FSMs cannot write LEDs or the display. */
    switch (active_mode)
    {
    case INPUT_MODE_NORMAL:
        fsm_automatic_run();
        break;
    case INPUT_MODE_RED:
    case INPUT_MODE_YELLOW:
    case INPUT_MODE_GREEN:
        fsm_setting_run(set_edge, save_edge);
        break;
    case INPUT_MODE_MANUAL:
        fsm_manual_run(set_edge);
        break;
    default:
        break;
    }
}
