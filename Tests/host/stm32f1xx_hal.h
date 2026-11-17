#ifndef MOCK_HAL_H
#define MOCK_HAL_H
#include <stdint.h>
typedef enum {GPIO_PIN_RESET=0,GPIO_PIN_SET=1} GPIO_PinState;
typedef struct {uint16_t odr,idr;} GPIO_TypeDef;
extern GPIO_TypeDef porta,portb;
#define GPIOA (&porta)
#define GPIOB (&portb)
extern uint32_t primask;
static inline uint32_t __get_PRIMASK(void){return primask;}
static inline void __disable_irq(void){primask=1;}
static inline void __set_PRIMASK(uint32_t p){primask=p;}
uint32_t HAL_GetTick(void);
GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef*,uint16_t);
void HAL_GPIO_WritePin(GPIO_TypeDef*,uint16_t,GPIO_PinState);
void HAL_GPIO_TogglePin(GPIO_TypeDef*,uint16_t);
#define GPIO_PIN_0 (1u << 0)
#define GPIO_PIN_1 (1u << 1)
#define GPIO_PIN_2 (1u << 2)
#define GPIO_PIN_3 (1u << 3)
#define GPIO_PIN_4 (1u << 4)
#define GPIO_PIN_5 (1u << 5)
#define GPIO_PIN_6 (1u << 6)
#define GPIO_PIN_7 (1u << 7)
#define GPIO_PIN_8 (1u << 8)
#define GPIO_PIN_9 (1u << 9)
#define GPIO_PIN_10 (1u << 10)
#define GPIO_PIN_11 (1u << 11)
#define GPIO_PIN_12 (1u << 12)
#define GPIO_PIN_13 (1u << 13)
#define GPIO_PIN_14 (1u << 14)
#define GPIO_PIN_15 (1u << 15)
typedef struct { uint16_t Pin; unsigned Mode, Pull; } GPIO_InitTypeDef;
#define GPIO_MODE_INPUT 0u
#define GPIO_PULLUP 1u
void HAL_GPIO_Init(GPIO_TypeDef*, GPIO_InitTypeDef*);
#endif
