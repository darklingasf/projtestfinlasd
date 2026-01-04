#include "spi.h"

#define SPI1EN     (1U<<12)
#define GPIOAEN    (1U<<0)

#define SR_TXE     (1U<<1)
#define SR_RXNE    (1U<<0)
#define SR_BSY     (1U<<7)

/*
 * SPI1 Pins
 * PA5 -> SCK
 * PA6 -> MISO
 * PA7 -> MOSI
 * PA9 -> CS
 */

void spi_gpio_init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB1ENR |= GPIOAEN;

    /* ---------- PA5,6,7 AF mode ---------- */
    GPIOA->MODER &= ~(0x3 << 10);  // PA5
    GPIOA->MODER |=  (0x2 << 10);

    GPIOA->MODER &= ~(0x3 << 12);  // PA6
    GPIOA->MODER |=  (0x2 << 12);

    GPIOA->MODER &= ~(0x3 << 14);  // PA7
    GPIOA->MODER |=  (0x2 << 14);

    /* ---------- PA9 output (CS) ---------- */
    GPIOA->MODER &= ~(0x3 << 18);
    GPIOA->MODER |=  (0x1 << 18);

    /* CS idle HIGH */
    GPIOA->ODR |= (1U << 9);

    /* ---------- Alternate Function SPI1 (AF5) ---------- */
    GPIOA->AFR[0] &= ~(0xFFF << 20);   // clear PA5,6,7
    GPIOA->AFR[0] |=  (5 << 20);       // PA5
    GPIOA->AFR[0] |=  (5 << 24);       // PA6
    GPIOA->AFR[0] |=  (5 << 28);       // PA7
}

void spi1_config(void)
{
    /* Enable SPI1 clock */
    RCC->APB2ENR |= SPI1EN;

    /* Disable SPI before config */
    SPI1->CR1 &= ~(1U << 6);

    /*
     * Baud rate: fPCLK / 8 (safe for ADXL345)
     * BR[2:0] = 010
     */
    SPI1->CR1 &= ~(0x7 << 3);
    SPI1->CR1 |=  (0x2 << 3);

    /* SPI Mode 3: CPOL=1, CPHA=1 */
    SPI1->CR1 |= (1U << 0);
    SPI1->CR1 |= (1U << 1);

    /* Full duplex */
    SPI1->CR1 &= ~(1U << 10);

    /* MSB first */
    SPI1->CR1 &= ~(1U << 7);

    /* Master mode */
    SPI1->CR1 |= (1U << 2);

    /* 8-bit data */
    SPI1->CR1 &= ~(1U << 11);

    /* Software slave management */
    SPI1->CR1 |= (1U << 8);
    SPI1->CR1 |= (1U << 9);

    /* Enable SPI */
    SPI1->CR1 |= (1U << 6);
}

void spi1_transmit(uint8_t *data, uint32_t size)
{
    uint32_t i = 0;
    volatile uint8_t temp;

    while (i < size)
    {
        while (!(SPI1->SR & SR_TXE)) {}
        SPI1->DR = data[i++];
    }

    while (!(SPI1->SR & SR_TXE)) {}
    while (SPI1->SR & SR_BSY) {}

    /* Clear OVR */
    temp = SPI1->DR;
    temp = SPI1->SR;
}

void spi1_receive(uint8_t *data, uint32_t size)
{
    volatile uint8_t temp;

    while (size)
    {
        while (!(SPI1->SR & SR_TXE)) {}
        SPI1->DR = 0x00;   // dummy clock

        while (!(SPI1->SR & SR_RXNE)) {}
        *data++ = SPI1->DR;
        size--;
    }

    while (SPI1->SR & SR_BSY) {}
    temp = SPI1->DR;
    temp = SPI1->SR;
}

void cs_enable(void)
{
    GPIOA->ODR &= ~(1U << 9);
}

void cs_disable(void)
{
    GPIOA->ODR |= (1U << 9);
}
