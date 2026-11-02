#include "timer.h"

TIM_HandleTypeDef htim2;
volatile uint32_t timer2_irq_count;

void timer2_init(void)
{
    TIM_ClockConfigTypeDef clock_source = {0};
    uint32_t timer_clock_hz = HAL_RCC_GetPCLK1Freq();

    /* APB1 timer clock is PCLK1 when /1, otherwise 2 * PCLK1. */
    if ((RCC->CFGR & RCC_CFGR_PPRE1) != 0u)
    {
        timer_clock_hz *= 2u;
    }

    /* Ex3: 10 kHz counter, 100 counts/update = 100 Hz = 10 ms.
     * Current HSI/APB1 = 8 MHz -> PSC = 799, ARR = 99.
     */
    if ((timer_clock_hz < 10000u) ||
        ((timer_clock_hz % 10000u) != 0u) ||
        ((timer_clock_hz / 10000u) > 65536u))
    {
        Error_Handler();
        return;
    }

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = (timer_clock_hz / 10000u) - 1u;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 99u;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        Error_Handler();
        return;
    }

    clock_source.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &clock_source) != HAL_OK)
    {
        Error_Handler();
        return;
    }

    /* Base init generates UG; discard its UIF so the first IRQ is at 10 ms. */
    __HAL_TIM_SET_COUNTER(&htim2, 0u);
    __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
    HAL_NVIC_ClearPendingIRQ(TIM2_IRQn);
    timer2_irq_count = 0u;
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
    {
        Error_Handler();
    }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();
        HAL_NVIC_SetPriority(TIM2_IRQn, 1u, 0u);
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        /* Minimal Ex3 probe; no button sampling or application logic yet. */
        timer2_irq_count++;
    }
}
