#ifndef SSD1306_H
#define SSD1306_H

#include <stdint.h>

#define SSD1306_ADDR 0x78

void SSD1306_Init(void);
void SSD1306_Update(void);
void SSD1306_Clear(void);
void SSD1306_PrintLabel(uint8_t page, uint8_t col, char* str);
void SSD1306_WriteInt(int16_t val, uint8_t page, uint8_t col);

#endif
