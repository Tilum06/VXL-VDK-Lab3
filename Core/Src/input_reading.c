#include "input_reading.h"
#include "main.h"

void input_reading_init(void)
{
    /* Ex5: initialize debounce buffers to released, clear hold counters.
     * Use MODE/SET/SAVE macros from main.h; do not reconfigure GPIO.
     * Active LOW buttons; Ex3 board_io_init enables internal pull-ups.
     */
}

void button_reading(void)
{
    /* Ex5: sample three buttons every 10 ms; publish debounced levels.
     * Measure hold time in milliseconds, not a fixed number of interrupts.
     * No delays, display scanning, or application FSM inside this function.
     */
}

uint8_t is_button_pressed(uint8_t index)
{
    (void)index;
    /* Ex5: validate index and return debounced level; currently inactive. */
    return 0u;
}

uint8_t is_button_pressed_1s(uint8_t index)
{
    (void)index;
    /* Ex5: validate index and return hold flag; currently inactive. */
    return 0u;
}
