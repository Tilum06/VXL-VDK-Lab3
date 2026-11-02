#include "output_display.h"
#include "main.h"

/* Bit 0..6 = A..G; table stores the active-LOW segment levels. */
static const uint8_t digit_segments[10] = {
    0x40u, 0x79u, 0x24u, 0x30u, 0x19u,
    0x12u, 0x02u, 0x78u, 0x00u, 0x10u
};
static const uint16_t segment_pins[7] = {
    SEG_A_Pin, SEG_B_Pin, SEG_C_Pin, SEG_D_Pin,
    SEG_E_Pin, SEG_F_Pin, SEG_G_Pin
};
static const uint16_t enable_pins[OUTPUT_DISPLAY_DIGIT_COUNT] = {
    EN0_Pin, EN1_Pin, EN2_Pin, EN3_Pin
};
#define ALL_ENABLE_PINS (EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin)
#define ALL_SEGMENT_PINS (SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin | \
                          SEG_E_Pin | SEG_F_Pin | SEG_G_Pin)
#define BLANK_FRAME 0x7f7f7f7fu

/* Writer and scanner both run in foreground. Latch at the frame boundary. */
static uint32_t pending_frame;
static uint32_t active_frame;
static uint32_t scan_tick_ms;
static uint8_t next_digit;

void output_display_init(void)
{
    HAL_GPIO_WritePin(GPIOB, ALL_ENABLE_PINS, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, ALL_SEGMENT_PINS, GPIO_PIN_SET);
    pending_frame = BLANK_FRAME;
    active_frame = BLANK_FRAME;
    next_digit = 0u;
    scan_tick_ms = HAL_GetTick();
}

void output_display_set_pairs(uint8_t first, uint8_t second)
{
    if ((first > 99u) || (second > 99u))
    {
        output_display_blank();
        return;
    }
    pending_frame = (uint32_t)digit_segments[first / 10u] |
                    ((uint32_t)digit_segments[first % 10u] << 8u) |
                    ((uint32_t)digit_segments[second / 10u] << 16u) |
                    ((uint32_t)digit_segments[second % 10u] << 24u);
}

void output_display_blank(void)
{
    pending_frame = BLANK_FRAME;
}

void output_display_scan(void)
{
    uint32_t now_ms = HAL_GetTick();
    if ((uint32_t)(now_ms - scan_tick_ms) < OUTPUT_DISPLAY_SCAN_PERIOD_MS)
    {
        return;
    }
    /* Skip missed slots rather than emitting a burst of invisible digits. */
    scan_tick_ms = now_ms;
    if (next_digit == 0u)
    {
        active_frame = pending_frame;
    }
    uint8_t segments = (uint8_t)(active_frame >> (8u * next_digit));
    uint16_t lit_pins = 0u;
    for (uint8_t i = 0u; i < 7u; i++)
    {
        if ((segments & (1u << i)) == 0u)
        {
            lit_pins |= segment_pins[i];
        }
    }

    /* Break before make: no digit enabled while segment data changes. */
    HAL_GPIO_WritePin(GPIOB, ALL_ENABLE_PINS, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, ALL_SEGMENT_PINS, GPIO_PIN_SET);
    if (lit_pins != 0u)
    {
        HAL_GPIO_WritePin(GPIOB, lit_pins, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(GPIOB, enable_pins[next_digit], GPIO_PIN_RESET);
    }
    next_digit = (next_digit + 1u) % OUTPUT_DISPLAY_DIGIT_COUNT;
}
