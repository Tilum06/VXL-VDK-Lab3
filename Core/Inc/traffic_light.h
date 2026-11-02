#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

#include <stdint.h>

/* 2 Hz full cycle = 500 ms; toggle every 250 ms. */
#define TRAFFIC_LIGHT_BLINK_HALF_PERIOD_MS 250u

void traffic_light_init(void);
/* Future foreground service with actual elapsed time, independent of tick size. */
void traffic_light_process(uint32_t elapsed_ms);

#endif /* INC_TRAFFIC_LIGHT_H_ */
