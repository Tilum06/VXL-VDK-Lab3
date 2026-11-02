#include "input_processing.h"
#include "input_reading.h"

typedef enum {
    MODE_WAIT_PRESS,
    MODE_WAIT_RELEASE
} ModeButtonState;

volatile InputMode input_mode = INPUT_MODE_NORMAL;
static ModeButtonState mode_button_state;

void input_processing_init(void)
{
    input_mode = INPUT_MODE_NORMAL;
    mode_button_state = MODE_WAIT_PRESS;
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
