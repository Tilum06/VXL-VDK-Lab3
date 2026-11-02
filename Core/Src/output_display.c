#include "output_display.h"
#include "main.h"

void output_display_init(void)
{
    /* Ex6: first disable EN0..EN3 (HIGH), then blank SEG_A..SEG_G (HIGH).
     * Common anode + PNP high-side: both EN and SEG are active LOW.
     * Use existing main.h macros: PB0..PB6 segments, PB7..PB10 enables.
     */
}

void output_display_set_pairs(uint8_t first, uint8_t second)
{
    (void)first;
    (void)second;
    /* Ex6: encode two 0..99 values into a coherent four-digit buffer.
     * Publish a complete frame safely if scanner and writer differ in context.
     */
}

void output_display_scan(void)
{
    /* Ex6: disable all digits -> set active-LOW segments -> enable one digit.
     * Use an independent fixed 1 ms timebase, including when TIM2 changes.
     * Do not scan four digits with delays, or scan once per 10 ms TIM2 tick.
     */
}
