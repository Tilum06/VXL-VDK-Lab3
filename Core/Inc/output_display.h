#ifndef INC_OUTPUT_DISPLAY_H_
#define INC_OUTPUT_DISPLAY_H_

#include <stdint.h>

#define OUTPUT_DISPLAY_DIGIT_COUNT 4u
#define OUTPUT_DISPLAY_SCAN_PERIOD_MS 1u

/* Call after GPIO init. All functions run in the foreground only. */
void output_display_init(void);
/* Values 0..99: EN0/1 = first pair, EN2/3 = second pair. */
void output_display_set_pairs(uint8_t first, uint8_t second);
/* Blank at the next frame boundary; invalid pair values also blank. */
void output_display_blank(void);
/* Call continuously: SysTick gates 1 ms/digit, 4 ms/frame (250 Hz). */
void output_display_scan(void);

#endif /* INC_OUTPUT_DISPLAY_H_ */
