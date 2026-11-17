#ifndef BUTTON_H
#define BUTTON_H
#include <stdint.h>
#define BUTTON_MODE 1u
#define BUTTON_SET  2u
#define BUTTON_SAVE 4u
void button_init(void);
void button_run(void);
uint8_t button_take_events(void);
#endif
