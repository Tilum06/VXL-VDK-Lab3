#include "input_processing.h"
#include "input_reading.h"
#include "output_display.h"
#include "traffic_light.h"
#include "main.h"

typedef enum {
    MODE_WAIT_PRESS,
    MODE_WAIT_RELEASE
} ModeButtonState;

volatile InputMode input_mode = INPUT_MODE_NORMAL;
static ModeButtonState mode_button_state;
static uint8_t edit_duration_s;
static uint8_t previous_set_pressed;
static uint8_t previous_save_pressed;
static uint8_t save_rejected;
static uint32_t save_error_tick_ms;

static uint8_t get_selected_duration_s(void)
{
    switch (input_mode)
    {
    case INPUT_MODE_YELLOW:
        return traffic_light_get_yellow_duration_s();
    case INPUT_MODE_GREEN:
        return traffic_light_get_green_duration_s();
    default:
        return traffic_light_get_red_duration_s();
    }
}

static void update_mode_display(void)
{
    switch (input_mode)
    {
    case INPUT_MODE_RED:
    case INPUT_MODE_YELLOW:
    case INPUT_MODE_GREEN:
        output_display_set_pairs((uint8_t)input_mode, save_rejected ? 0u : edit_duration_s);
        break;
    default:
        /* traffic_light_process owns the normal-mode countdown display. */
        break;
    }
}

void input_processing_init(void)
{
    input_mode = INPUT_MODE_NORMAL;
    mode_button_state = MODE_WAIT_PRESS;
    edit_duration_s = get_selected_duration_s();
    previous_set_pressed = 0u;
    previous_save_pressed = 0u;
    save_rejected = 0u;
    update_mode_display();
}

void fsm_for_input_processing(void)
{
    uint8_t pressed = is_button_pressed(INPUT_BUTTON_MODE);
    uint8_t set_pressed = is_button_pressed(INPUT_BUTTON_SET);
    uint8_t save_pressed = is_button_pressed(INPUT_BUTTON_SAVE);
    uint8_t set_edge = set_pressed && !previous_set_pressed;
    uint8_t save_edge = save_pressed && !previous_save_pressed;
    InputMode previous_mode = input_mode;

    /* Track edges in every mode: holding a button while entering
     * edit mode must not create a new press or repeat action.
     */
    previous_set_pressed = set_pressed;
    previous_save_pressed = save_pressed;

    switch (mode_button_state)
    {
    case MODE_WAIT_PRESS:
        if (pressed != 0u)
        {
            input_mode = (input_mode == INPUT_MODE_GREEN) ? INPUT_MODE_NORMAL :
                         (InputMode)(input_mode + 1);
            mode_button_state = MODE_WAIT_RELEASE;
        }
        break;
    case MODE_WAIT_RELEASE:
        if (pressed == 0u)
        {
            mode_button_state = MODE_WAIT_PRESS;
        }
        break;
    default:
        mode_button_state = MODE_WAIT_PRESS;
        break;
    }

    if (input_mode != previous_mode)
    {
        /* Enter from the confirmed value; leaving discards unsaved edits.
         * Mode changes take priority over simultaneous SET/SAVE edges.
         */
        edit_duration_s = get_selected_duration_s();
        save_rejected = 0u;
        update_mode_display();
        return;
    }

    if ((input_mode == INPUT_MODE_RED) || (input_mode == INPUT_MODE_YELLOW) ||
        (input_mode == INPUT_MODE_GREEN))
    {
        if (set_edge)
        {
            save_rejected = 0u;
            edit_duration_s = (edit_duration_s >= INPUT_DURATION_MAX_S) ?
                              INPUT_DURATION_MIN_S : edit_duration_s + 1u;
        }
        if (save_edge)
        {
            uint8_t accepted;
            /* If SET and SAVE arrive together, save the incremented value. */
            if (input_mode == INPUT_MODE_RED)
            {
                accepted = traffic_light_set_red_duration_s(edit_duration_s);
            }
            else if (input_mode == INPUT_MODE_YELLOW)
            {
                accepted = traffic_light_set_yellow_duration_s(edit_duration_s);
            }
            else
            {
                accepted = traffic_light_set_green_duration_s(edit_duration_s);
            }
            save_rejected = !accepted;
            save_error_tick_ms = HAL_GetTick();
        }
        uint8_t error_expired = save_rejected &&
            ((uint32_t)(HAL_GetTick() - save_error_tick_ms) >= 1000u);
        if (error_expired) save_rejected = 0u;
        if (set_edge || save_edge || error_expired)
        {
            update_mode_display();
        }
    }
}
