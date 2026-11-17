#include "traffic_light.h"
#include "main.h"

void traffic_light_set(Light road1, Light road2)
{
    static const uint16_t pins1[] = {0, RED_LED_Pin, YELLOW_LED_Pin, GREEN_LED_Pin};
    static const uint16_t pins2[] = {0, RED_LED2_Pin, YELLOW_LED2_Pin, GREEN_LED2_Pin};
    const uint16_t all = RED_LED_Pin | YELLOW_LED_Pin | GREEN_LED_Pin |
                         RED_LED2_Pin | YELLOW_LED2_Pin | GREEN_LED2_Pin;
    /* Setting deliberately blinks both greens; Automatic never requests that. */
    HAL_GPIO_WritePin(GPIOA, all, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, pins1[road1] | pins2[road2], GPIO_PIN_RESET);
}
