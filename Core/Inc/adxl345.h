#ifndef ADXL345_H_
#define ADXL345_H_

#include "spi.h"
#include <stdint.h>

/* ================= Registers ================= */

#define ADXL345_REG_DEVID             0x00
#define ADXL345_REG_THRESH_ACT        0x24
#define ADXL345_REG_DUR               0x21
#define ADXL345_REG_ACT_INACT_CTL     0x27
#define ADXL345_REG_BW_RATE           0x2C   // <-- MISSING ONE
#define ADXL345_REG_POWER_CTL         0x2D
#define ADXL345_REG_INT_ENABLE        0x2E
#define ADXL345_REG_INT_MAP           0x2F
#define ADXL345_REG_INT_SOURCE        0x30
#define ADXL345_REG_DATA_FORMAT       0x31
#define ADXL345_REG_DATA_START        0x32

/* ================= Bit Masks ================= */

/* DATA_FORMAT */
#define ADXL345_RANGE_4G              0x01
#define ADXL345_FULL_RES              (1 << 3)

/* POWER_CTL */
#define ADXL345_RESET                 0x00
#define ADXL345_MEASURE_BIT           (1 << 3)

/* ACT_INACT_CTL (Activity only) */
#define ADXL345_ACT_X_ENABLE          (1 << 6)
#define ADXL345_ACT_Y_ENABLE          (1 << 5)
#define ADXL345_ACT_Z_ENABLE          (1 << 4)
#define ADXL345_ACT_ACDC              (1 << 7)   /* 1 = AC coupling */

/* INT bits */
#define ADXL345_INT_ACTIVITY          (1 << 4)

/* SPI protocol bits */
#define ADXL345_MULTI_BYTE_ENABLE     0x40
#define ADXL345_READ_OPERATION        0x80

/* ================= API ================= */

/* Initialize ADXL345 (SPI, range, measurement mode) */
void adxl_init(void);

/* Low-level SPI register access */
void adxl_read(uint8_t address, uint8_t *rxdata, uint8_t len);
void adxl_write(uint8_t address, uint8_t data);

/* Read XYZ acceleration */
void adxl_read_xyz(int16_t *ax, int16_t *ay, int16_t *az);

/* Enable activity interrupt on INT1 */
void adxl_enable_activity_interrupt(uint8_t threshold);

/* Clear interrupt latch (read INT_SOURCE) */
void adxl_clear_interrupt(void);

#endif /* ADXL345_H_ */
