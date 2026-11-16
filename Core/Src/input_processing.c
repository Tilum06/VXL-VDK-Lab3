#include "input_processing.h"
#include "button.h"
#include "fsm_automatic.h"
#include "fsm_setting.h"
#include "fsm_manual.h"
#include "software_timer.h"
#include "output_display.h"
#include "main.h"

static InputMode active_mode;
static uint8_t pending_set, pending_save;
static uint32_t button_sample_ms;

void input_processing_init(void)
{
    input_mode = INPUT_MODE_NORMAL;
    active_mode = (InputMode)0;
    pending_set = pending_save = 0u;
    button_sample_ms = HAL_GetTick();
}

void fsm_for_input_processing(void)
{
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

    pending_set = set_edge;
    pending_save = save_edge;
}

void Task_Button(void)
{
    uint32_t now = HAL_GetTick();
    /* Backlog is not historical GPIO data: never fabricate debounce samples
     * by reading the same level several times in a fast catch-up loop.
     */
    if ((uint32_t)(now - button_sample_ms) < 10u) return;
    button_sample_ms = now;
    button_reading();
}

void Task_SoftwareTimer(void)
{
    timerRun();
}

void Task_ModeController(void)
{
    fsm_for_input_processing();
}

void Task_Automatic(void)
{
    if (active_mode == INPUT_MODE_NORMAL) fsm_automatic_run();
}

void Task_Manual(void)
{
    if (active_mode == INPUT_MODE_MANUAL)
    {
        uint8_t set_edge = pending_set;
        pending_set = 0u;
        fsm_manual_run(set_edge);
    }
}

void Task_Setting(void)
{
    if (active_mode >= INPUT_MODE_RED && active_mode <= INPUT_MODE_GREEN)
    {
        uint8_t set_edge = pending_set;
        uint8_t save_edge = pending_save;
        pending_set = pending_save = 0u;
        fsm_setting_run(set_edge, save_edge);
    }
}
