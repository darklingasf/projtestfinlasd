#include "tim.h"

/**
 * @brief Simple software delay
 * Note: For 16MHz HSI, 3195 is an approximate multiplier for 1ms.
 */
void delay_ms(uint32_t ms) {
    for (volatile uint32_t i = 0; i < ms * 3195; i++) {
        __NOP(); // No Operation: prevents compiler from optimizing out the loop
    }
}

/**
 * @brief Optional: Initialize TIM2 for hardware-based timing
 * Useful if you want to implement Requirement R4 (5-second logging) precisely.
 */
void TIM2_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Set for 1ms update at 16MHz
    TIM2->PSC = 16000 - 1;
    TIM2->ARR = 1000 - 1;

    TIM2->CR1 |= TIM_CR1_CEN;
}
