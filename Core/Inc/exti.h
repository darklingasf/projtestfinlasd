#ifndef EXTI_H
#define EXTI_H

#include "stm32f4xx.h"

// Connect ADXL INT1 to PB0
#define ADXL_INT_PIN 0

void EXTI_Init(void);
uint8_t EXTI_GetFlag(void);
void EXTI_ClearFlag(void);

#endif
