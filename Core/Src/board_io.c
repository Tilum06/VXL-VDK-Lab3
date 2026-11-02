#include "board_io.h"
#include "main.h"

void board_io_init(void)
{
    GPIO_InitTypeDef input = {0};

    /* Disable PNP digit enables before blanking the active-LOW segments. */
    HAL_GPIO_WritePin(GPIOB, EN0_Pin | EN1_Pin | EN2_Pin | EN3_Pin,
                      GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, SEG_A_Pin | SEG_B_Pin | SEG_C_Pin | SEG_D_Pin |
                      SEG_E_Pin | SEG_F_Pin | SEG_G_Pin, GPIO_PIN_SET);

    /* CubeMX currently generates GPIO_NOPULL. Apply the Ex3 pull-ups here. */
    input.Pin = MODE_Pin | SET_Pin | SAVE_Pin;
    input.Mode = GPIO_MODE_INPUT;
    input.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &input);
}
