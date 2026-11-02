#include "input_processing.h"
#include "input_reading.h"
#include "output_display.h"

/* Ex6 placeholders only; SET/SAVE editing belongs to Ex7-Ex9. */
#define INITIAL_RED_DURATION_S 5u
#define INITIAL_YELLOW_DURATION_S 2u
#define INITIAL_GREEN_DURATION_S 3u

typedef enum {
    MODE_WAIT_PRESS,
    MODE_WAIT_RELEASE
} ModeButtonState;

volatile InputMode input_mode = INPUT_MODE_NORMAL;
static ModeButtonState mode_button_state;

static void update_mode_display(void)
{
    switch (input_mode)
    {
    case INPUT_MODE_RED:
        output_display_set_pairs(INPUT_MODE_RED, INITIAL_RED_DURATION_S);
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
    update_mode_display();
}

void fsm_for_input_processing(void)
{
    uint8_t pressed = is_button_pressed(INPUT_BUTTON_MODE);

    switch (mode_button_state)
    {
    case MODE_WAIT_PRESS:
        if (pressed != 0u)
        {
            input_mode = (input_mode == INPUT_MODE_GREEN) ? INPUT_MODE_NORMAL :
                         (InputMode)(input_mode + 1);
            mode_button_state = MODE_WAIT_RELEASE;
            update_mode_display();
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
}
