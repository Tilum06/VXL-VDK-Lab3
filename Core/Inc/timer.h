#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "main.h"

extern TIM_HandleTypeDef htim2;
/* Debugger/Proteus watch: advances by approximately 100 each second. */
extern volatile uint32_t timer2_irq_count;

/* Ex3 only: internal-clock TIM2, 10 ms update interrupt. */
void timer2_init(void);

#endif /* INC_TIMER_H_ */
