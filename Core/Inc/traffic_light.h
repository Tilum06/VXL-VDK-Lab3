#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

#include <stdint.h>

/* Optional diagnostic sequence; normal builds run the Ex6 mode logic. */
#ifndef TRAFFIC_LED_TEST
#define TRAFFIC_LED_TEST 0
#endif

#define LED_ON GPIO_PIN_RESET
#define LED_OFF GPIO_PIN_SET

/* 2 Hz full cycle = 500 ms; toggle every 250 ms. */
#define TRAFFIC_LIGHT_BLINK_HALF_PERIOD_MS 250u

void traffic_light_init(void);
/* Confirmed red duration in RAM, 1..99 seconds; reset restores 5 seconds.
 * The normal traffic/countdown FSM is not implemented yet.
 */
uint8_t traffic_light_get_red_duration_s(void);
void traffic_light_set_red_duration_s(uint8_t duration_s);
/* Active-LOW Ex6 mode blink using Ex4's HAL tick; call in foreground. */
void traffic_light_process(void);

#endif /* INC_TRAFFIC_LIGHT_H_ */
