#include "tim.h"
#include "adxl345.h"
#include "log.h"

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

    /* Reset counter */
    TIM2->CNT = 0;

    /* Enable update interrupt */
    TIM2->DIER |= (1U << 0);

    /* Enable TIM2 interrupt */
    NVIC_EnableIRQ(TIM2_IRQn);
}

void tim2_start(void)
{
    TIM2->CNT = 0;
    TIM2->CR1 |= (1U << 0);   // CEN
}

void tim2_stop(void)
{
    TIM2->CR1 &= ~(1U << 0);
}

/* ---------- TIM2 ISR ---------- */
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & (1U << 0))   // UIF
    {
        /* Clear update flag */
        TIM2->SR &= ~(1U << 0);

        if (!log_is_active())
        {
            tim2_stop();
            return;
        }

        int16_t ax, ay, az;

        /* Fast SPI burst */
        adxl_read_xyz(&ax, &ay, &az);

        log_add_sample(ax, ay, az);

        /* Stop exactly at 100 samples */
        if (log_is_complete())
        {
            tim2_stop();
        }
    }
}
