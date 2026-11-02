#include "timer.h"
#include "input_reading.h"

volatile uint32_t timer2_irq_count;

void timer2_init(void)
{
    /* MX_TIM2_Init owns the handle, clock, PSC/ARR and MSP/NVIC setup.
     * Keep CubeMX settings so regeneration can select 1, 10 or 100 ms.
     * Discard init's UIF so the first IRQ follows one complete period.
     */
    __HAL_TIM_SET_COUNTER(&htim2, 0u);
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
    HAL_NVIC_ClearPendingIRQ(TIM2_IRQn);
    timer2_irq_count = 0u;
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        timer2_irq_count++;
        button_reading();
    }
}
