#include "timer.h"
#include "scheduler.h"


void timer2_init(void)
{
    /* HSI/APB1 = 8 MHz, PSC=799, ARR=99: 8e6/800/100 = 100 Hz.
     * Discard init's UIF so the first IRQ follows one complete 10 ms period.
     */
    __HAL_TIM_SET_COUNTER(&htim2, 0u);
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
    HAL_NVIC_ClearPendingIRQ(TIM2_IRQn);
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        SCH_Update();
    }
}
