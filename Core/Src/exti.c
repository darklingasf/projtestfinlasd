#include "exti.h"

static volatile uint8_t shock = 0;

void EXTI_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    SYSCFG->EXTICR[0] &= ~(0xF); // Port B for EXTI0
    EXTI->IMR |= (1 << ADXL_INT_PIN);
    EXTI->RTSR |= (1 << ADXL_INT_PIN);
    NVIC_EnableIRQ(EXTI0_IRQn);
}

uint8_t EXTI_GetFlag(void) { return shock; }
void EXTI_ClearFlag(void) { shock = 0; }

void EXTI0_IRQHandler(void) {
    if(EXTI->PR & (1 << ADXL_INT_PIN)) {
        EXTI->PR |= (1 << ADXL_INT_PIN);
        shock = 1;
    }
}
