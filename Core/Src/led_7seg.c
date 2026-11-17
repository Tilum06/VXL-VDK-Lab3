#include "led_7seg.h"
#include "main.h"
#define DIGITS (EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin)
#define SEGMENTS (SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | \
                  SEG_E_Pin | SEG_F_Pin | SEG_G_Pin)
static const uint8_t codes[10] = {
    0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x78, 0x00, 0x10
};
static const uint16_t enables[4] = {EN0_Pin, EN1_Pin, EN2_Pin, EN3_Pin};
static uint8_t buffer[4], digit, enabled;
static uint32_t last_scan;

void led_7seg_init(void)
{
    digit = enabled = 0;
    last_scan = HAL_GetTick();
    led_7seg_blank();
}

void led_7seg_show(uint8_t road1, uint8_t road2)
{
    buffer[0] = (road1 / 10) % 10;
    buffer[1] = road1 % 10;
    buffer[2] = (road2 / 10) % 10;
    buffer[3] = road2 % 10;
    enabled = 1;
}

void led_7seg_blank(void)
{
    enabled = 0;
    HAL_GPIO_WritePin(GPIOB, DIGITS | SEGMENTS, GPIO_PIN_SET);
}

void led_7seg_scan(void)
{
    uint32_t now = HAL_GetTick();
    if ((uint32_t)(now - last_scan) < 2u) return;
    last_scan = now;
    HAL_GPIO_WritePin(GPIOB, DIGITS, GPIO_PIN_SET);
    if (!enabled) return;
    /* Disable all digits before changing the segment bus (no ghosting). */
    HAL_GPIO_WritePin(GPIOB, SEGMENTS, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, (uint16_t)(~codes[buffer[digit]]) & SEGMENTS,
                      GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, enables[digit], GPIO_PIN_RESET);
    digit = (digit + 1u) % 4u;
}
