#ifndef INC_TRAFFIC_LIGHT_H_
#define INC_TRAFFIC_LIGHT_H_

#include <stdint.h>

#define LED_ON GPIO_PIN_RESET
#define LED_OFF GPIO_PIN_SET

/* 2 Hz full cycle = 500 ms; toggle every 250 ms. */
#define TRAFFIC_LIGHT_BLINK_HALF_PERIOD_MS 250u

void traffic_light_init(void);
/* RAM defaults red/yellow/green = 5/2/3 seconds. Setters return 1 on success,
 * 0 without changes if outside 1..99 or red < green + yellow.
 * Normal traffic snapshots the saved values on mode entry.
 */
uint8_t traffic_light_get_red_duration_s(void);
uint8_t traffic_light_set_red_duration_s(uint8_t duration_s);
uint8_t traffic_light_get_yellow_duration_s(void);
uint8_t traffic_light_set_yellow_duration_s(uint8_t duration_s);
uint8_t traffic_light_get_green_duration_s(void);
uint8_t traffic_light_set_green_duration_s(uint8_t duration_s);
/* Foreground: normal traffic/countdown or selected-color 2 Hz blink. */
void traffic_light_process(void);

#endif /* INC_TRAFFIC_LIGHT_H_ */
