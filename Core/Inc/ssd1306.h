#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>
#include <stdbool.h>
#include "i2c_dma.h"

/* SSD1306 display size */
#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  64
#define SSD1306_PAGES   (SSD1306_HEIGHT/8)

/* Framebuffer */
extern uint8_t ssd1306_buffer[SSD1306_WIDTH * SSD1306_PAGES];

/* Basic API */
void ssd1306_init(void);
void ssd1306_update(void);           // send framebuffer via DMA
void ssd1306_clear(void);
void ssd1306_set_cursor(uint8_t x, uint8_t page);
void ssd1306_draw_char(char c);
void ssd1306_draw_string(const char *str);

/* Higher-level helpers */
void ssd1306_show_monitoring(int16_t pitch, int16_t roll);
void ssd1306_show_shock_detected(void);

/* Fonts */
extern const uint8_t Font6x8[][6];

#endif
