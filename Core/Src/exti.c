#include "exti.h"
#include "adxl345.h"

/*
 * ADXL345 INT1 connected to PA8 → EXTI8
 */

volatile uint8_t adxl_int_flag = 0;

void adxl_exti_init(void)
{
    /* 1. Enable GPIOA clock */
    RCC->AHB1ENR |= (1U << 0);

    /* 2. Configure PA8 as input */
    GPIOA->MODER &= ~(3U << (8 * 2));

    /* 3. Enable SYSCFG clock */
    RCC->APB2ENR |= (1U << 14);

    /* 4. Map EXTI8 to PA8 */
    SYSCFG->EXTICR[2] &= ~(0xF << 0);

    /* 5. Unmask EXTI8 */
    EXTI->IMR |= (1U << 8);

    /* 6. Rising edge trigger */
    EXTI->RTSR |= (1U << 8);
    EXTI->FTSR &= ~(1U << 8);

    /* 7. Enable EXTI9_5 interrupt */
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* -------- EXTI ISR -------- */
void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & (1U << 8))
    {
        /* Clear EXTI pending bit (write 1 to clear) */
        EXTI->PR = (1U << 8);

        /* Clear ADXL345 interrupt latch */
        adxl_clear_interrupt();

        /* Signal motion detected */
        adxl_int_flag = 1;
    }
}
