#ifndef ADXL345_H
#define ADXL345_H

#include <stdint.h>

#define ADXL_ADDR_READ    0x80
#define ADXL_ADDR_MB      0x40
#define ADXL_REG_THRESH   0x1D
#define ADXL_REG_DATAX0   0x32
#define ADXL_REG_INT_MAP  0x2F
#define ADXL_REG_INT_EN   0x2E

void ADXL_Init(void);
void ADXL_EnableInterrupt(uint8_t threshold);
void ADXL_ReadRaw(int16_t *x, int16_t *y, int16_t *z);
int16_t ADXL_CalculateAngle(int16_t y, int16_t x);

#endif
