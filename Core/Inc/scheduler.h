#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>

#define SCH_MAX_TASKS 6u
#define SCH_INVALID_TASK_ID 0u

typedef struct {
    void (*pTask)(void);
    uint32_t Delay;
    uint32_t Period;
    uint32_t RunMe;
    uint32_t TaskID;
} sTask;

/* Sticky overload indication: pending releases saturate instead of wrapping. */
extern volatile uint8_t SCH_Overflow;

/* Init before starting TIM2. Foreground owns Add/Delete/Dispatch; TIM2 calls
 * Update. APIs preserve PRIMASK; Dispatch defers work if IRQs are disabled.
 */
void SCH_Init(void);
/* Tick units (10 ms). Delay=0 queues an immediate release. Period=0: one-shot.
 * Returns a unique nonzero ID, or SCH_INVALID_TASK_ID for NULL/full/ID exhaustion.
 */
uint32_t SCH_Add_Task(void (*task)(void), uint32_t delay, uint32_t period);
uint8_t SCH_Delete_Task(uint32_t task_id);
void SCH_Update(void);
/* Foreground only, non-reentrant. One release per slot per call, in slot order.
 * Tasks may add/delete tasks. Deleted/replaced releases will not be dispatched.
 */
void SCH_Dispatch_Tasks(void);

#endif
