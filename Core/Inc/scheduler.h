#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <stdint.h>
#define SCH_MAX_TASKS 10u
#define SCH_INVALID_TASK_ID 0u
typedef struct {
    void (*pTask)(void);
    uint32_t Delay, Period, RunMe, TaskID;
} sTask;
void SCH_Init(void);
uint32_t SCH_Add_Task(void (*task)(void), uint32_t delay, uint32_t period);
uint8_t SCH_Delete_Task(uint32_t id);
void SCH_Update(void);
void SCH_Dispatch_Tasks(void);
#endif
