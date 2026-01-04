#include "tim.h"
#include <stdint.h>

volatile uint8_t oled_update_flag = 0;

/* TIM2 @ 20 Hz, SYSCLK = 16 MHz */
void tim2_init(void)
{
    RCC->APB1ENR |= (1U << 0);   // TIM2 clock
    TIM2->CR1 &= ~(1U << 0);     // Disable during setup

    TIM2->PSC = 1600 - 1;        // 16 MHz → 10 kHz
    TIM2->ARR = 500 - 1;         // 50 ms → 20 Hz
    TIM2->CNT = 0;

    TIM2->DIER |= TIM_DIER_UIE;  // Enable update interrupt
    NVIC_EnableIRQ(TIM2_IRQn);
}

void tim2_start(void) { TIM2->CR1 |= TIM_CR1_CEN; }
void tim2_stop(void)  { TIM2->CR1 &= ~TIM_CR1_CEN; }

void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF;
        oled_update_flag = 1;
    }
}
