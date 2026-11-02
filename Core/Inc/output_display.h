#ifndef INC_OUTPUT_DISPLAY_H_
#define INC_OUTPUT_DISPLAY_H_

#include <stdint.h>

#define OUTPUT_DISPLAY_DIGIT_COUNT 4u
#define OUTPUT_DISPLAY_SCAN_PERIOD_MS 1u

/* Skeleton only: none of these functions currently writes GPIO. */
void output_display_init(void);
/* Future buffer update: EN0/1 = first pair, EN2/3 = second pair. */
void output_display_set_pairs(uint8_t first, uint8_t second);
/* Future fixed 1 ms service: one digit per call, 4 ms per complete frame. */
void output_display_scan(void);

#endif /* INC_OUTPUT_DISPLAY_H_ */
