#include "adxl345.h"

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

/* ================= Core Init ================= */

void adxl_init(void)
{
    spi_gpio_init();
    spi1_config();

    /* --- Force standby first --- */
    adxl_write(ADXL345_REG_POWER_CTL, 0x00);

    /* --- Data rate = 100 Hz (safe for activity detect) --- */
    adxl_write(ADXL345_REG_BW_RATE, 0x0A);

    /* --- ±4g, full resolution, INT active high --- */
    adxl_write(ADXL345_REG_DATA_FORMAT,
               ADXL345_RANGE_4G |
               ADXL345_FULL_RES |
               0x00);   // INT_INVERT = 0

    /* --- Enter measurement mode --- */
    adxl_write(ADXL345_REG_POWER_CTL, ADXL345_MEASURE_BIT);
}

/* ================= Data Read ================= */

void adxl_read_xyz(int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t raw[6];
    adxl_read(ADXL345_REG_DATA_START, raw, 6);

    *ax = (int16_t)((raw[1] << 8) | raw[0]);
    *ay = (int16_t)((raw[3] << 8) | raw[2]);
    *az = (int16_t)((raw[5] << 8) | raw[4]);
}

/* ================= Interrupt Config ================= */

void adxl_enable_activity_interrupt(uint8_t threshold)
{
    /* Disable all interrupts */
    adxl_write(ADXL345_REG_INT_ENABLE, 0x00);

    /* Activity threshold (62.5 mg/LSB) */
    adxl_write(ADXL345_REG_THRESH_ACT, threshold);

    /* Duration = 2 * 625 µs */
    adxl_write(ADXL345_REG_DUR, 2);

    /* DC-coupled activity on XYZ */
    adxl_write(ADXL345_REG_ACT_INACT_CTL,
               ADXL345_ACT_X_ENABLE |
               ADXL345_ACT_Y_ENABLE |
               ADXL345_ACT_Z_ENABLE);

    /* Map Activity → INT1 */
    adxl_write(ADXL345_REG_INT_MAP, 0x00);

    /* Enable Activity interrupt */
    adxl_write(ADXL345_REG_INT_ENABLE, ADXL345_INT_ACTIVITY);

    /* Clear latch AFTER enabling */
    adxl_clear_interrupt();
}

void adxl_clear_interrupt(void)
{
    uint8_t tmp;
    adxl_read(ADXL345_REG_INT_SOURCE, &tmp, 1);
}
