#include "i2c_dma.h"
#include "stm32f4xx.h"

/* DMA transfer active flag */
static volatile bool dma_busy = false;

void i2c_dma_init(void)
{
    /* --- Enable clocks --- */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;  // PB8=SCL, PB9=SDA
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

    /* --- Configure GPIO alternate function --- */
    GPIOB->MODER   &= ~((3<<16)|(3<<18));
    GPIOB->MODER   |=  ((2<<16)|(2<<18));
    GPIOB->OTYPER  |=  ((1<<8)|(1<<9));   // Open-drain
    GPIOB->OSPEEDR |=  ((3<<16)|(3<<18));
    GPIOB->AFR[1]  |= ((4<<0)|(4<<4));    // AF4 I2C1

    /* --- I2C1 configuration --- */
    I2C1->CR1 = (1<<15); // reset
    I2C1->CR1 = 0;
    I2C1->CR2 = 16;      // APB1 = 16 MHz
    I2C1->CCR = 80;      // 100 kHz
    I2C1->TRISE = 17;
    I2C1->CR1 |= (1<<0); // enable I2C

    /* --- DMA1 Stream6 for I2C1_TX --- */
    DMA1_Stream6->CR = 0;
    DMA1_Stream6->FCR = 0;
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}

/* Start DMA transfer of framebuffer */
int i2c_dma_send(uint8_t *data, uint16_t size)
{
    // start DMA transfer
    // return 0 if successful, -1 if not
}


bool i2c_dma_ready(void)
{
    return !dma_busy;
}

/* DMA completion ISR */
void DMA1_Stream6_IRQHandler(void)
{
    if (DMA1->HISR & (1<<21)) // TCIF6
    {
        /* Clear flags */
        DMA1->HIFCR |= (1<<21) | (1<<20) | (1<<19) | (1<<18);

        /* Disable DMA stream */
        DMA1_Stream6->CR &= ~(1<<0);

        /* Stop I2C transfer */
        I2C1->CR1 |= (1<<9); // STOP

        dma_busy = false;
    }
}
