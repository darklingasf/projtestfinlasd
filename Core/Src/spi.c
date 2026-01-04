#include "spi.h"

void SPI1_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    GPIOA->MODER |= (2 << 10) | (2 << 12) | (2 << 14); // PA5,6,7 AF
    GPIOA->AFR[0] |= (5 << 20) | (5 << 24) | (5 << 28); // AF5
    GPIOA->MODER |= (1 << (ADXL_CS_PIN * 2)); // PA9 Output

    SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_BR_2 | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_SSM | SPI_CR1_SSI;
    SPI1->CR1 |= SPI_CR1_SPE;
    SPI_CS_High();
}

uint8_t SPI_Xfer(uint8_t data) {
    SPI1->DR = data;
    while(!(SPI1->SR & SPI_SR_RXNE));
    return SPI1->DR;
}

void SPI_CS_Low(void) { GPIOA->ODR &= ~(1 << ADXL_CS_PIN); }
void SPI_CS_High(void) { GPIOA->ODR |= (1 << ADXL_CS_PIN); }
