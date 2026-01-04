#include "stm32f4xx.h"
#include "i2c_dma.h"

void I2C1_DMA_Init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_DMA1EN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB->MODER |= (2 << 16) | (2 << 18); // PB8, PB9 AF
    GPIOB->OTYPER |= (1 << 8) | (1 << 9);  // Open Drain
    GPIOB->AFR[1] |= (4 << 0) | (4 << 4);  // AF4

    I2C1->CR1 |= I2C_CR1_SWRST;
    for(volatile int i=0; i<100; i++);
    I2C1->CR1 &= ~I2C_CR1_SWRST;

    I2C1->CR2 = 16;  // 16MHz
    I2C1->CCR = 80;  // 100kHz
    I2C1->TRISE = 17;
    I2C1->CR1 |= I2C_CR1_PE;

    // DMA1 Stream 6 Channel 1 (I2C1_TX)
    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while(DMA1_Stream6->CR & DMA_SxCR_EN);
    DMA1_Stream6->PAR = (uint32_t)&I2C1->DR;
    DMA1_Stream6->CR = (1 << 25) | DMA_SxCR_MINC | DMA_SxCR_DIR_0;
}

uint8_t I2C1_IsBusy(void) {
    // Check if I2C bus is busy OR DMA is still enabling/transferring
    return (I2C1->SR2 & I2C_SR2_BUSY) || (DMA1_Stream6->NDTR != 0);
}

void I2C1_DMA_Write(uint8_t slave_addr, uint8_t control_byte, uint8_t *pData, uint16_t size) {
    while(I2C1->SR2 & I2C_SR2_BUSY); // Wait for bus

    I2C1->CR1 |= I2C_CR1_START;
    while(!(I2C1->SR1 & I2C_SR1_SB));

    I2C1->DR = slave_addr;
    while(!(I2C1->SR1 & I2C_SR1_ADDR));
    (void)I2C1->SR2; // Mandatory: Read SR2 to clear ADDR flag

    I2C1->DR = control_byte;
    while(!(I2C1->SR1 & I2C_SR1_TXE));

    if(size > 0) {
        DMA1->HIFCR = DMA_HIFCR_CTCIF6; // Clear DMA flags
        DMA1_Stream6->M0AR = (uint32_t)pData;
        DMA1_Stream6->NDTR = size;
        I2C1->CR2 |= I2C_CR2_DMAEN;
        DMA1_Stream6->CR |= DMA_SxCR_EN;
        // Wait for DMA to complete to prevent overlapping writes
        while(DMA1_Stream6->NDTR != 0);
    }

    // Ensure last byte finished before STOP
    while(!(I2C1->SR1 & I2C_SR1_BTF));
    I2C1->CR1 |= I2C_CR1_STOP;
}
