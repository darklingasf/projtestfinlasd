#include "uart.h"
#include "stm32f411xe.h"
#include "adxl345.h"
#include "log.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* ================= CONFIG ================= */
#define SYSCLK_FREQ   16000000UL
#define UART_BAUDRATE 115200
#define RX_BUF_SIZE   64

/* ================= RX STATE ================= */
static volatile char rx_buf[RX_BUF_SIZE];
static volatile uint8_t rx_idx = 0;
static volatile uint8_t cmd_ready = 0;
volatile uint8_t uart_get_log_received = 0;
extern volatile uint8_t capture_armed;
extern volatile uint8_t oled_frozen;
extern void oled_show_monitoring(void);  // for resetting OLED

/* ================= LOW LEVEL INIT ================= */
static void gpio_uart_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* PA2 TX, PA3 RX -> AF7 */
    GPIOA->MODER &= ~((3<<4)|(3<<6));
    GPIOA->MODER |=  ((2<<4)|(2<<6));

    GPIOA->AFR[0] &= ~((0xF<<8)|(0xF<<12));
    GPIOA->AFR[0] |=  ((7<<8)|(7<<12));

    GPIOA->OSPEEDR |= (3<<4)|(3<<6);
}

static void usart2_init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    USART2->BRR = SYSCLK_FREQ / UART_BAUDRATE;

    USART2->CR1 |= USART_CR1_RE | USART_CR1_TE; // RX & TX
    USART2->CR1 |= USART_CR1_RXNEIE;            // RX interrupt
    USART2->CR1 |= USART_CR1_UE;                // enable USART

    NVIC_EnableIRQ(USART2_IRQn);
}

/* ================= PUBLIC INIT ================= */
void uart_init(void)
{
    gpio_uart_init();
    usart2_init();
    uart_send_string("Hello UART!\r\n");
}

/* ================= SEND FUNCTIONS ================= */
void uart_send_char(char c)
{
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
}

void uart_send_string(const char *str)
{
    while (*str)
        uart_send_char(*str++);
}

/* ================= RX IRQ ================= */
void USART2_IRQHandler(void)
{
    if (USART2->SR & USART_SR_RXNE)
    {
        char c = USART2->DR;

        if (c == '\r' || c == '\n')
        {
            rx_buf[rx_idx] = '\0';
            rx_idx = 0;
            cmd_ready = 1;
        }
        else if (rx_idx < RX_BUF_SIZE-1)
        {
            rx_buf[rx_idx++] = c;
        }
    }
}

/* ================= COMMAND PARSER ================= */
void uart_process(void)
{
    if (!cmd_ready) return;
    cmd_ready = 0;

    /* ---- SET_THRESH <value> ---- */
    if (strncmp((char*)rx_buf, "SET_THRESH", 10) == 0)
    {
        char *arg = strchr((char*)rx_buf, ' ');
        if (arg)
        {
            uint16_t val = (uint16_t)atoi(arg + 1);
            uart_cmd_set_thresh(val);
            uart_send_string("OK\r\n");
        }
        else
        {
            uart_send_string("ERR\r\n");
        }
    }
    /* ---- GET_LOG ---- */
    else if (strcmp((char*)rx_buf, "GET_LOG") == 0)
    {
        uart_get_log_received = 1;      // flag for main loop
        uart_cmd_get_log();              // send log and handle END_LOG rearm
    }
    else
    {
        uart_send_string("UNKNOWN CMD\r\n");
    }
}

/* ================= DEFAULT WEAK IMPLEMENTATIONS ================= */
__attribute__((weak)) void uart_cmd_set_thresh(uint16_t value)
{
    adxl_enable_activity_interrupt(value);
    uart_send_string("Threshold updated\r\n");
}

__attribute__((weak)) void uart_cmd_get_log(void)
{
    char line[64];
    uart_send_string("AX,AY,AZ\r\n");

    const log_sample_t *buf = log_get_buffer();
    uint16_t count = log_get_count();

    for (uint16_t i = 0; i < count; i++)
    {
        snprintf(line, sizeof(line), "%d,%d,%d\r\n",
                 buf[i].ax, buf[i].ay, buf[i].az);
        uart_send_string(line);
    }

    uart_send_string("END_LOG\r\n");

    /* ----- REARM SYSTEM AFTER LOG ----- */
    log_reset();                  // clear log buffer
    capture_armed = 1;            // allow new shock
    EXTI->IMR |= (1U << 8);       // re-enable ADXL345 interrupt
    oled_frozen = 0;              // unfreeze OLED
    // optional: restore default monitoring display
    extern void oled_show_monitoring(void);
    oled_show_monitoring();

    uart_send_string("LOG COMPLETED, EXTI REARMED, OLED RESET\r\n");
}
