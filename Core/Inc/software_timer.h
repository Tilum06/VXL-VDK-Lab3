#ifndef SOFTWARE_TIMER_H
#define SOFTWARE_TIMER_H
#include <stdint.h>
#define TIMER_TICK_MS 10u
typedef enum {
    TIMER_AUTO_PHASE, TIMER_MANUAL, TIMER_BLINK, TIMER_ERROR, TIMER_COUNT
} TimerID;
void software_timer_init(void);
void software_timer_run(void);
void setTimer(TimerID id, uint32_t ms);
uint8_t isTimerExpired(TimerID id);
uint32_t timerRemaining(TimerID id);
#endif
