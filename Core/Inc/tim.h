#ifndef TIM_H
#define TIM_H

#include "stm32f4xx.h"

void TIM2_Init(void); // 1ms base timer
void delay_ms(uint32_t ms);
uint32_t HAL_GetTick(void);

#endif
