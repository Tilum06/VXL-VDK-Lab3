#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "main.h"

extern TIM_HandleTypeDef htim2;
/* Debug probe only: 1000/100/10 IRQs per second at 1/10/100 ms. */
extern volatile uint32_t timer2_irq_count;

/* Call after MX_TIM2_Init: start interrupts without overriding PSC/ARR. */
void timer2_init(void);

#endif /* INC_TIMER_H_ */
