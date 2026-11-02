#include "traffic_light.h"
#include "main.h"

void traffic_light_init(void)
{
    /* Ex1: define normal traffic phases and valid initial durations.
     * Keep RED_LED/YELLOW_LED/GREEN_LED on PA5..PA7 and road 2 on PA8..PA10.
     * Traffic LED polarity must be checked against the actual schematic.
     */
}

void traffic_light_process(uint32_t elapsed_ms)
{
    (void)elapsed_ms;
    /* Ex4: accumulate elapsed_ms; for each elapsed 250 ms toggle blink phase
     * and subtract 250 ms, preserving the remainder (do not reset to zero).
     * Coarse ticks cause edge jitter; they must not cause cumulative drift.
     * Ex6: blink both roads' selected color in edit mode.
     * Ex10: add coordinated traffic phases/countdowns and display updates.
     * This skeleton intentionally contains no running traffic/blink logic.
     */
}
