#include "stm32f411xe.h"
#include "adxl345.h"
#include "uart.h"

/* Declare externally defined in main.c */
extern volatile uint8_t shock_detected;
extern volatile uint8_t capture_armed;  // make sure main.c has this variable

/* ---------- EXTI Initialization ---------- */
void adxl_exti_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER &= ~(3U << (8*2));        // PA8 input
    GPIOA->PUPDR &= ~(3U << (8*2));
    GPIOA->PUPDR |=  (2U << (8*2));        // pull-down

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[2] &= ~(0xF << 0);      // PA8 -> EXTI8

    EXTI->IMR  |= (1U << 8);               // unmask
    EXTI->RTSR |= (1U << 8);               // rising edge
    EXTI->FTSR &= ~(1U << 8);              // no falling edge

    NVIC_SetPriority(EXTI9_5_IRQn, 2);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* ---------- ISR ---------- */
void EXTI9_5_IRQHandler(void)
{
    if (EXTI->PR & (1U << 8))
    {
        adxl_clear_interrupt();   // clear sensor latch
        EXTI->PR = (1U << 8);     // clear EXTI pending

        // Only trigger if capture is armed
        if (capture_armed)
        {
            shock_detected = 1;  // <-- main loop uses this
            uart_send_string("INTERRUPT DETECTED\r\n");
        }
    }
}
