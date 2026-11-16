#include "scheduler.h"
#include "main.h"

static volatile sTask tasks[SCH_MAX_TASKS];
static uint32_t next_id;
volatile uint8_t SCH_Overflow;

static uint32_t lock(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static void clear_slot(uint32_t index)
{
    tasks[index].pTask = 0;
    tasks[index].Delay = 0u;
    tasks[index].Period = 0u;
    tasks[index].RunMe = 0u;
    tasks[index].TaskID = SCH_INVALID_TASK_ID;
}

void SCH_Init(void)
{
    uint32_t primask = lock();
    for (uint32_t i = 0u; i < SCH_MAX_TASKS; ++i) clear_slot(i);
    next_id = 0u;
    SCH_Overflow = 0u;
    __set_PRIMASK(primask);
}

uint32_t SCH_Add_Task(void (*task)(void), uint32_t delay, uint32_t period)
{
    if (task == 0) return SCH_INVALID_TASK_ID;
    /* Only foreground callers mutate slot ownership. */
    for (uint32_t i = 0u; i < SCH_MAX_TASKS; ++i)
    {
        uint32_t primask = lock();
        if (tasks[i].pTask == 0 && next_id != UINT32_MAX)
        {
            uint32_t id = ++next_id;
            tasks[i].Delay = (delay == 0u) ? period : delay;
            tasks[i].Period = period;
            tasks[i].RunMe = (delay == 0u) ? 1u : 0u;
            tasks[i].TaskID = id;
            tasks[i].pTask = task;
            __set_PRIMASK(primask);
            return id;
        }
        __set_PRIMASK(primask);
    }
    return SCH_INVALID_TASK_ID;
}

uint8_t SCH_Delete_Task(uint32_t task_id)
{
    if (task_id == SCH_INVALID_TASK_ID) return 0u;
    for (uint32_t i = 0u; i < SCH_MAX_TASKS; ++i)
    {
        uint32_t primask = lock();
        if (tasks[i].TaskID == task_id)
        {
            clear_slot(i);
            __set_PRIMASK(primask);
            return 1u;
        }
        __set_PRIMASK(primask);
    }
    return 0u;
}

void SCH_Update(void)
{
    for (uint32_t i = 0u; i < SCH_MAX_TASKS; ++i)
    {
        uint32_t primask = lock();
        if (tasks[i].pTask != 0)
        {
            /* A queued one-shot must never be queued again before dispatch. */
            if (tasks[i].Period != 0u || tasks[i].RunMe == 0u)
            {
                if (tasks[i].Delay > 0u) --tasks[i].Delay;
                if (tasks[i].Delay == 0u)
                {
                    if (tasks[i].RunMe != UINT32_MAX) ++tasks[i].RunMe;
                    else SCH_Overflow = 1u;
                    tasks[i].Delay = tasks[i].Period;
                }
            }
        }
        __set_PRIMASK(primask);
    }
}

void SCH_Dispatch_Tasks(void)
{
    /* Defer rather than invoke application code in a caller's critical section. */
    if (__get_PRIMASK() != 0u) return;
    uint32_t ready[SCH_MAX_TASKS];
    /* One bounded snapshot prevents an IRQ halfway through a pass from making
     * later FSM tasks run ahead of the Button/ModeController for that release.
     */
    uint32_t primask = lock();
    for (uint32_t i = 0u; i < SCH_MAX_TASKS; ++i)
        ready[i] = tasks[i].RunMe ? tasks[i].TaskID : SCH_INVALID_TASK_ID;
    __set_PRIMASK(primask);

    for (uint32_t i = 0u; i < SCH_MAX_TASKS; ++i)
    {
        void (*task)(void) = 0;
        primask = lock();
        if (ready[i] != SCH_INVALID_TASK_ID && tasks[i].TaskID == ready[i] &&
            tasks[i].RunMe != 0u)
        {
            task = tasks[i].pTask;
            --tasks[i].RunMe;
            if (tasks[i].Period == 0u) clear_slot(i);
        }
        __set_PRIMASK(primask);
        if (task != 0) task();
    }
}
