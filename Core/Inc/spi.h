#ifndef SPI_H
#define SPI_H

#include "stm32f4xx.h"

#define ADXL_CS_PIN 9 // PA9

void SPI1_Init(void);
uint8_t SPI_Xfer(uint8_t data);
void SPI_CS_Low(void);
void SPI_CS_High(void);

#endif
