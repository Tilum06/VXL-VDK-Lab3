#ifndef INC_TIMER_H_
#define INC_TIMER_H_

#include "main.h"

extern TIM_HandleTypeDef htim2;
/* Call after MX_TIM2_Init: start interrupts without overriding PSC/ARR. */
void timer2_init(void);

#endif /* INC_TIMER_H_ */
