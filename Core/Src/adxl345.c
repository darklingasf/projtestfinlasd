#include "adxl345.h"
#include "spi.h"

void ADXL_Init(void) {
    SPI1_Init();
    SPI_CS_Low(); SPI_Xfer(0x2D); SPI_Xfer(0x08); SPI_CS_High(); // Measure mode
}

void ADXL_EnableInterrupt(uint8_t threshold) {
    SPI_CS_Low(); SPI_Xfer(ADXL_REG_THRESH); SPI_Xfer(threshold); SPI_CS_High();
    SPI_CS_Low(); SPI_Xfer(0x27); SPI_Xfer(0x70); SPI_CS_High(); // ACT XYZ
    SPI_CS_Low(); SPI_Xfer(ADXL_REG_INT_MAP); SPI_Xfer(0x00); SPI_CS_High(); // All to INT1
    SPI_CS_Low(); SPI_Xfer(ADXL_REG_INT_EN); SPI_Xfer(0x10); SPI_CS_High(); // Activity
}

void ADXL_ReadRaw(int16_t *x, int16_t *y, int16_t *z) {
    SPI_CS_Low();
    SPI_Xfer(ADXL_REG_DATAX0 | ADXL_ADDR_READ | ADXL_ADDR_MB);
    uint8_t b[6];
    for(int i=0; i<6; i++) b[i] = SPI_Xfer(0x00);
    SPI_CS_High();
    *x = (int16_t)(b[1]<<8 | b[0]); *y = (int16_t)(b[3]<<8 | b[2]); *z = (int16_t)(b[5]<<8 | b[4]);
}

int16_t ADXL_CalculateAngle(int16_t y, int16_t x) {
    if (x == 0 && y == 0) return 0;
    int16_t angle = (y >= 0) ? 45 : -45; // Basic integer approximation for demo
    return angle;
}
