#ifndef LED_7SEG_H
#define LED_7SEG_H
#include <stdint.h>
void led_7seg_init(void);
void led_7seg_show(uint8_t road1, uint8_t road2);
void led_7seg_blank(void);
void led_7seg_scan(void);
#endif
