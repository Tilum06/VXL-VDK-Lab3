#include "software_timer.h"
static uint32_t remaining[TIMER_COUNT];

void software_timer_init(void)
{
    for (unsigned i = 0; i < TIMER_COUNT; ++i) remaining[i] = 0;
}

void software_timer_run(void)
{
    for (unsigned i = 0; i < TIMER_COUNT; ++i)
        if (remaining[i] > 0) --remaining[i];
}

void setTimer(TimerID id, uint32_t ms)
{
    if (id < TIMER_COUNT)
        remaining[id] = ms / TIMER_TICK_MS + (ms % TIMER_TICK_MS != 0);
}

uint8_t isTimerExpired(TimerID id)
{
    /* Level, not a consumed edge; a zero-duration timer is already expired. */
    return id < TIMER_COUNT && remaining[id] == 0;
}

uint32_t timerRemaining(TimerID id)
{
    return id < TIMER_COUNT ? remaining[id] * TIMER_TICK_MS : 0;
}
