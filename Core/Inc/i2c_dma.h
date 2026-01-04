#ifndef I2C_DMA_H
#define I2C_DMA_H

#include "stm32f4xx.h"

void I2C1_DMA_Init(void);
void I2C1_DMA_Write(uint8_t slave_addr, uint8_t control_byte, uint8_t *pData, uint16_t size);
uint8_t I2C1_IsBusy(void);

#endif
