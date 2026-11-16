#include "software_timer.h"
#include "main.h"

typedef struct {
    uint32_t start_ms;
    uint32_t duration_ms;
    uint8_t active;
    uint8_t expired;
} SoftwareTimer;

static SoftwareTimer timers[SOFTWARE_TIMER_COUNT];
static uint32_t now_ms;

static uint8_t valid_id(SoftwareTimerId id)
{
    return (uint32_t)id < SOFTWARE_TIMER_COUNT;
}

void stopTimer(SoftwareTimerId id)
{
    if (!valid_id(id)) return;
    timers[id].active = 0u;
    timers[id].expired = 0u;
}

void stopAllTimers(void)
{
    for (uint8_t i = 0u; i < SOFTWARE_TIMER_COUNT; i++) stopTimer((SoftwareTimerId)i);
}

void software_timer_init(void)
{
    now_ms = HAL_GetTick();
    stopAllTimers();
}

void timerRun(void)
{
    now_ms = HAL_GetTick();
    for (uint8_t i = 0u; i < SOFTWARE_TIMER_COUNT; i++)
    {
        SoftwareTimer *timer = &timers[i];
        if (timer->active && (uint32_t)(now_ms - timer->start_ms) >= timer->duration_ms)
            timer->expired = 1u;
    }
}

void setTimerElapsed(SoftwareTimerId id, uint32_t duration_ms, uint32_t elapsed_ms)
{
    if (!valid_id(id)) return;
    timers[id].start_ms = now_ms - elapsed_ms;
    timers[id].duration_ms = duration_ms;
    timers[id].expired = elapsed_ms >= duration_ms;
    timers[id].active = 1u;
}

void setTimer(SoftwareTimerId id, uint32_t duration_ms)
{
    setTimerElapsed(id, duration_ms, 0u);
}

uint8_t isTimerActive(SoftwareTimerId id)
{
    return valid_id(id) && timers[id].active;
}

uint8_t isTimerExpired(SoftwareTimerId id)
{
    return isTimerActive(id) && timers[id].expired;
}

uint32_t getTimerElapsed(SoftwareTimerId id)
{
    if (!isTimerActive(id)) return 0u;
    return now_ms - timers[id].start_ms;
}

uint32_t getTimerRemaining(SoftwareTimerId id)
{
    if (!isTimerActive(id) || isTimerExpired(id)) return 0u;
    return timers[id].duration_ms - getTimerElapsed(id);
}

uint32_t consumeTimerPeriods(SoftwareTimerId id)
{
    if (!isTimerActive(id) || timers[id].duration_ms == 0u) return 0u;
    uint32_t periods = getTimerElapsed(id) / timers[id].duration_ms;
    timers[id].start_ms += periods * timers[id].duration_ms;
    timers[id].expired = 0u;
    return periods;
}
