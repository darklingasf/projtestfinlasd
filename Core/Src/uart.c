#include "stm32f4xx.h"  // <--- This defines USART2 and its registers
#include "uart.h"
#include <string.h>

static char rx_buffer[50];
static uint8_t rx_index = 0;
static uint8_t command_ready = 0;

void UART2_Init(void) {
    // Enable GPIOA and USART2 Clocks
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // PA2 (TX), PA3 (RX) as Alternate Function (AF7)
    GPIOA->MODER &= ~((3 << 4) | (3 << 6));
    GPIOA->MODER |= (2 << 4) | (2 << 6);
    GPIOA->AFR[0] |= (7 << 8) | (7 << 12);

    // 9600 Baud @ 16MHz (16,000,000 / (16 * 9600) = 104.18 -> 104 / 3)
    USART2->BRR = 0x0683;

    // Enable Transmit, Receive, and Receive Interrupt
    USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;
    USART2->CR1 |= USART_CR1_UE;

    // Enable NVIC for USART2 Interrupt
    NVIC_EnableIRQ(USART2_IRQn);
}

void UART2_SendString(char* str) {
    while(*str) {
        // Use generic USART_SR_TXE
        while(!(USART2->SR & USART_SR_TXE));
        USART2->DR = *str++;
    }
}

uint8_t UART2_GetCommand(char* buffer) {
    if(!command_ready) return 0;
    strcpy(buffer, rx_buffer);
    command_ready = 0;
    return 1;
}

// ISR must match the name in the startup file
void USART2_IRQHandler(void) {
    // Use generic USART_SR_RXNE
    if(USART2->SR & USART_SR_RXNE) {
        char c = (char)USART2->DR;
        if(c == '\n' || c == '\r') {
            rx_buffer[rx_index] = '\0';
            if(rx_index > 0) command_ready = 1; // Only signal if string is not empty
            rx_index = 0;
        } else {
            if (rx_index < 49) {
                rx_buffer[rx_index++] = c;
            }
        }
    }
}
