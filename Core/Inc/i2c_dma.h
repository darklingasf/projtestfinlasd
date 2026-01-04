#ifndef I2C_DMA_H
#define I2C_DMA_H

#include <stdint.h>
#include <stdbool.h>

#define OLED_I2C_ADDR  0x3C

/* Initialize I2C1 + DMA for SSD1306 */
void i2c_dma_init(void);

/* Send framebuffer to SSD1306 via DMA (non-blocking) */
int i2c_dma_send(uint8_t *data, uint16_t size);

/* Returns true if previous DMA transfer is complete */
bool i2c_dma_ready(void);

#endif
