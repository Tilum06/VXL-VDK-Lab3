#include "scheduler.h"
#include "main.h"

/* Packed in insertion order, never sorted by Delay. Only Update runs in ISR. */
static volatile sTask tasks[SCH_MAX_TASKS];
static volatile uint32_t task_count;
static uint32_t next_id;

void SCH_Init(void)
{
    uint32_t irq = __get_PRIMASK();
    __disable_irq();
    task_count = 0;
    next_id = 1;
    __set_PRIMASK(irq);
}

uint32_t SCH_Add_Task(void (*task)(void), uint32_t delay, uint32_t period)
{
    uint32_t irq = __get_PRIMASK();
    uint32_t id = SCH_INVALID_TASK_ID;
    __disable_irq();
    if (task != 0 && task_count < SCH_MAX_TASKS && next_id != 0) {
        id = next_id++;
        tasks[task_count++] = (sTask){task, delay == 0 ? period : delay,
                                    period, delay == 0, id};
    }
    __set_PRIMASK(irq);
    return id;
}

uint8_t SCH_Delete_Task(uint32_t id)
{
    uint32_t irq = __get_PRIMASK();
    uint8_t found = 0;
    __disable_irq();
    for (uint32_t i = 0; i < task_count; ++i) {
        if (tasks[i].TaskID == id) {
            for (uint32_t j = i + 1; j < task_count; ++j)
                tasks[j - 1] = tasks[j];
            --task_count;
            found = 1;
            break;
        }
    }
    __set_PRIMASK(irq);
    return found;
}

void SCH_Update(void)
{
    for (uint32_t i = 0; i < task_count; ++i) {
        if (tasks[i].Delay > 0) {
            --tasks[i].Delay;
            if (tasks[i].Delay == 0) {
                ++tasks[i].RunMe;
                tasks[i].Delay = tasks[i].Period;
            }
        }
    }
}

void SCH_Dispatch_Tasks(void)
{
    void (*ready[SCH_MAX_TASKS])(void);
    uint32_t ready_count = 0, kept = 0;
    uint32_t irq = __get_PRIMASK();
    __disable_irq();
    /* Claim one release per task as a batch. An IRQ halfway through dispatch
     * cannot make a later task overtake an earlier one from the same tick.
     * Compact completed one-shots in this same O(N) pass.
     */
    for (uint32_t i = 0; i < task_count; ++i) {
        if (tasks[i].RunMe > 0) {
            ready[ready_count++] = tasks[i].pTask;
            --tasks[i].RunMe;
            if (tasks[i].Period == 0) continue;
        }
        tasks[kept++] = tasks[i];
    }
    task_count = kept;
    __set_PRIMASK(irq);
    /* Already claimed callbacks finish this batch even if a callback deletes
     * a task. Add/Delete from callbacks affects the following dispatch batch.
     */
    for (uint32_t i = 0; i < ready_count; ++i) ready[i]();
}
