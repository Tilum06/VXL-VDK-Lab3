#include "input_processing.h"
#include "input_reading.h"
#include "output_display.h"
#include "traffic_light.h"

/* Yellow/green editing remains outside Ex7. */
#define INITIAL_YELLOW_DURATION_S 2u
#define INITIAL_GREEN_DURATION_S 3u

typedef enum {
    MODE_WAIT_PRESS,
    MODE_WAIT_RELEASE
} ModeButtonState;

volatile InputMode input_mode = INPUT_MODE_NORMAL;
static ModeButtonState mode_button_state;
static uint8_t red_edit_duration_s;
static uint8_t previous_set_pressed;
static uint8_t previous_save_pressed;

static void update_mode_display(void)
{
    switch (input_mode)
    {
    case INPUT_MODE_RED:
        output_display_set_pairs(INPUT_MODE_RED, red_edit_duration_s);
        break;
    case INPUT_MODE_YELLOW:
        output_display_set_pairs(INPUT_MODE_YELLOW, INITIAL_YELLOW_DURATION_S);
        break;
    case INPUT_MODE_GREEN:
        output_display_set_pairs(INPUT_MODE_GREEN, INITIAL_GREEN_DURATION_S);
        break;
    default:
        /* No traffic countdown exists yet in normal mode. */
        output_display_blank();
        break;
    }
}

void input_processing_init(void)
{
    input_mode = INPUT_MODE_NORMAL;
    mode_button_state = MODE_WAIT_PRESS;
    red_edit_duration_s = traffic_light_get_red_duration_s();
    previous_set_pressed = 0u;
    previous_save_pressed = 0u;
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

    /* Track edges even outside Mode 2: holding a button while entering
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
        red_edit_duration_s = traffic_light_get_red_duration_s();
        update_mode_display();
        return;
    }

    if (input_mode == INPUT_MODE_RED)
    {
        if (set_edge)
        {
            red_edit_duration_s = (red_edit_duration_s >= INPUT_DURATION_MAX_S) ?
                                  INPUT_DURATION_MIN_S : red_edit_duration_s + 1u;
        }
        if (save_edge)
        {
            /* If SET and SAVE arrive together, save the incremented value. */
            traffic_light_set_red_duration_s(red_edit_duration_s);
        }
        if (set_edge || save_edge)
        {
            update_mode_display();
        }
    }
}
