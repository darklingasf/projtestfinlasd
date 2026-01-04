#include "exti.h"
#include "adxl345.h"
#include "uart.h"

extern volatile uint8_t oled_update_flag;
extern volatile uint8_t adxl_int_flag;
/*
 * ADXL345 INT1 → PA8 → EXTI8
 */

volatile uint8_t adxl_int_flag = 0;
extern volatile uint8_t shock_detected;

void EXTIx_IRQHandler(void)
{
    if (EXTI->PR & EXTI_PR_PR8)
    {
        EXTI->PR = EXTI_PR_PR8;
        shock_detected = 1;
    }
}

void adxl_exti_init(void)
{
    /* 1. Enable GPIOA clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* 2. PA8 input */
    GPIOA->MODER &= ~(3U << (8 * 2));

    /* 3. Pull-down (IMPORTANT) */
    GPIOA->PUPDR &= ~(3U << (8 * 2));
    GPIOA->PUPDR |=  (2U << (8 * 2));   // pull-down

    /* 4. Enable SYSCFG */
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    /* 5. EXTI8 mapped to PA8 */
    SYSCFG->EXTICR[2] &= ~(0xF << 0);

    /* 6. Unmask EXTI8 */
    EXTI->IMR |= (1U << 8);

    /* 7. Rising edge trigger */
    EXTI->RTSR |= (1U << 8);
    EXTI->FTSR &= ~(1U << 8);

    /* 8. NVIC enable */
    NVIC_SetPriority(EXTI9_5_IRQn, 2);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* -------- ISR -------- */
void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & (1U << 8))
    {
        EXTI->PR = (1U << 8);

        adxl_clear_interrupt();

        adxl_int_flag = 1;   // <<< THIS
        uart_send_string("INTERRUPT DETECTED\r\n");
    }
}
