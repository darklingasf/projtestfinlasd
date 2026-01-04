#include "adxl345.h"

/******** PINOUT ********/
/* STM32        ADXL345
 * PA6 (MISO) → SDO
 * PA7 (MOSI) → SDA
 * PA5 (SCK)  → SCL
 * PA9 (CS)   → CS
 * PA8 (EXTI) → INT1
 * GND        → GND
 * 3.3V       → VCC
 */

/* ================= Low-level SPI ================= */

void adxl_read(uint8_t address, uint8_t *rxdata, uint8_t len)
{
    address |= ADXL345_READ_OPERATION;

    if (len > 1)
        address |= ADXL345_MULTI_BYTE_ENABLE;

    cs_enable();
    spi1_transmit(&address, 1);
    spi1_receive(rxdata, len);
    cs_disable();
}

void adxl_write(uint8_t address, uint8_t value)
{
    uint8_t data[2] = { address, value };

    cs_enable();
    spi1_transmit(data, 2);
    cs_disable();
}

/* ================= High-level API ================= */

void adxl_init(void)
{
    /* Enable SPI GPIO and SPI peripheral */
    spi_gpio_init();
    spi1_config();

    /* ±4g, full resolution */
    adxl_write(ADXL345_REG_DATA_FORMAT,
               ADXL345_RANGE_4G | ADXL345_FULL_RES);

    /* Reset power control */
    adxl_write(ADXL345_REG_POWER_CTL, ADXL345_RESET);

    /* Measurement mode */
    adxl_write(ADXL345_REG_POWER_CTL, ADXL345_MEASURE_BIT);
}

void adxl_read_xyz(int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t raw[6];

    adxl_read(ADXL345_REG_DATA_START, raw, 6);

    *ax = (int16_t)((raw[1] << 8) | raw[0]);
    *ay = (int16_t)((raw[3] << 8) | raw[2]);
    *az = (int16_t)((raw[5] << 8) | raw[4]);
}

void adxl_enable_activity_interrupt(uint8_t threshold)
{
    /* Threshold: 1 LSB = 62.5 mg */
    adxl_write(ADXL345_REG_THRESH_ACT, threshold);

    /* Duration: 1 LSB = 625 µs */
    adxl_write(ADXL345_REG_DUR, 2);

    /* Enable activity detection on XYZ, AC coupled */
    adxl_write(ADXL345_REG_ACT_INACT_CTL,
               ADXL345_ACT_X_ENABLE |
               ADXL345_ACT_Y_ENABLE |
               ADXL345_ACT_Z_ENABLE |
               ADXL345_ACT_ACDC);

    /* Enable Activity interrupt */
    adxl_write(ADXL345_REG_INT_ENABLE, ADXL345_INT_ACTIVITY);

    /* Map Activity interrupt to INT1 */
    adxl_write(ADXL345_REG_INT_MAP, 0x00);
}

void adxl_clear_interrupt(void)
{
    uint8_t src;
    adxl_read(ADXL345_REG_INT_SOURCE, &src, 1);
}
