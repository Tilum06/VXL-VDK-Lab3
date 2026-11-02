#include "input_processing.h"
#include "input_reading.h"
#include "output_display.h"
#include "traffic_light.h"

void input_processing_init(void)
{
    /* Ex1/5: initialize normal mode and previous button levels. */
}

void fsm_for_input_processing(void)
{
    /* Ex5: MODE press edge cycles 1 -> 2 -> 3 -> 4 -> 1.
     * Ex6: publish mode and edit value to the display buffer.
     * Ex7/8/9: SET edits 1..99; SAVE commits red/yellow/green respectively.
     * Keep pending values separate from committed traffic durations.
     * Ex10: integrate normal traffic FSM only after earlier exercises.
     */
}
