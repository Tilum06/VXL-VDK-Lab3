#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include <stdint.h>

typedef enum {
    TIMER_AUTO_PHASE,
    TIMER_AUTO_ALL_RED,
    TIMER_SETTING_BLINK,
    TIMER_SETTING_ERROR,
    TIMER_MANUAL_TIMEOUT,
    TIMER_MANUAL_ENTRY,
    SOFTWARE_TIMER_COUNT
} SoftwareTimerId;

/* Foreground-only. timerRun samples HAL tick once before dispatching FSMs.
 * Unsigned elapsed subtraction supports tick wrap; update within one full
 * uint32_t tick cycle (about 49.7 days). No dependence on TIM2's period.
 */
void software_timer_init(void);
void timerRun(void);
void setTimer(SoftwareTimerId id, uint32_t duration_ms);
/* Restore elapsed time, preserving a phase's overshoot or an all-red handoff. */
void setTimerElapsed(SoftwareTimerId id, uint32_t duration_ms, uint32_t elapsed_ms);
void stopTimer(SoftwareTimerId id);
void stopAllTimers(void);
uint8_t isTimerActive(SoftwareTimerId id);
/* Sticky, non-consuming flag, cleared by stop/rearm. Zero duration expires now. */
uint8_t isTimerExpired(SoftwareTimerId id);
uint32_t getTimerRemaining(SoftwareTimerId id);
uint32_t getTimerElapsed(SoftwareTimerId id);
/* Periodic timer: consume all elapsed periods, retaining the fractional part.
 * Zero-duration/inactive timers return zero (never divide by zero).
 */
uint32_t consumeTimerPeriods(SoftwareTimerId id);

#endif
