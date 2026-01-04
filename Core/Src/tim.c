#include "tim.h"
#include "adxl345.h"
#include "log.h"
#include <stdint.h>

volatile uint8_t oled_update_flag = 0;

/*
 * TIM2 @ 20 Hz
 * SYSCLK = 16 MHz
 */


void tim2_init(void)
{
    /* Enable TIM2 clock */
    RCC->APB1ENR |= (1U << 0);

    /* Disable timer during setup */
    TIM2->CR1 &= ~(1U << 0);

    /*
     * Timer clock = 16 MHz
     * Prescaler = 1600 → 10 kHz
     */
    TIM2->PSC = 1600 - 1;

    /*
     * Auto-reload = 500 → 50 ms → 20 Hz
     */
    TIM2->ARR = 500 - 1;

    TIM2->CNT = 0;

    /* Enable update interrupt */
    TIM2->DIER |= (1U << 0);

    /* Enable TIM2 IRQ */
    NVIC_EnableIRQ(TIM2_IRQn);
}

void tim2_start(void)
{
    TIM2->CNT = 0;
    TIM2->CR1 |= (1U << 0);   // CEN
}

void tim2_stop(void)
{
    TIM2->CR1 &= ~(1U << 0);  // CEN = 0
}

/* ---------- TIM2 ISR ---------- */
volatile uint8_t tim2_flag = 0;

void TIM2_IRQHandler(void)
{
    if (TIM2->SR & TIM_SR_UIF)
    {
        TIM2->SR &= ~TIM_SR_UIF;
        tim2_flag = 1;
    }
}
