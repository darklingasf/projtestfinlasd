#include "stm32f4xx.h"
#include "ssd1306.h"
#include "i2c_dma.h"
#include "ssd1306_fonts.h"
#include "uart.h"
#include "tim.h"
#include <string.h>

static uint8_t SSD1306_Buffer[1024];

void SSD1306_Init(void) {
    UART2_SendString("OLED: Starting Init...\r\n");
    I2C1_DMA_Init();
    delay_ms(200);

    uint8_t init_cmds[] = {
        0xAE,       // Display Off
        0xD5, 0x80, // Set Clock Divide Ratio
        0xA8, 0x3F, // Multiplex Ratio
        0xD3, 0x00, // Display Offset
        0x40,       // Start Line
        0x8D, 0x14, // Charge Pump (Enable)
        0x20, 0x00, // Horizontal Addressing
        0xA1,       // Segment Remap
        0xC8,       // COM Scan Direction
        0xDA, 0x12, // COM Pins hardware config
        0x81, 0x7F, // Contrast (Reduced to 0x7F to lower power draw during test)
        0xD9, 0x22, // Pre-charge (Standard 0x22)
        0xDB, 0x20, // VCOMH Deselect level
        0xA4,       // Resume RAM to display
        0xA6,       // Normal Display (not inverted)
        0xAF        // Display ON
    };

    for(int i = 0; i < sizeof(init_cmds); i++) {
        I2C1_DMA_Write(SSD1306_ADDR, 0x00, &init_cmds[i], 1);
        while(I2C1_IsBusy());
    }

    SSD1306_Clear();
    SSD1306_Update();
    UART2_SendString("OLED: Init Sequence Complete.\r\n");
}

void SSD1306_Clear(void) {
    memset(SSD1306_Buffer, 0, 1024);
}

void SSD1306_Update(void) {
    // 1. Tell the OLED where we are writing (Manual/Blocking for stability)
    uint8_t addr_cmds[] = {0x21, 0, 127, 0x22, 0, 7};
    for(int i = 0; i < 6; i++) {
        I2C1_DMA_Write(SSD1306_ADDR, 0x00, &addr_cmds[i], 1);
        while(I2C1_IsBusy()); // Wait for each command to land
    }

    // 2. Blast the 1024 bytes of pixel data via DMA
    // 0x40 is the 'Data' stream prefix
    I2C1_DMA_Write(SSD1306_ADDR, 0x40, SSD1306_Buffer, 1024);

    // IMPORTANT: Wait for DMA to finish before allowing the MCU to do anything else
    // This prevents "tearing" or flickering.
    while(I2C1_IsBusy());
}

// THE FUNCTION THAT WAS CAUSING THE ERROR:
void SSD1306_PrintLabel(uint8_t page, uint8_t col, char* str) {
    uint8_t start_col = col;

    while(*str && col < 128) {  // Check against screen width (128 columns)
        // Check ASCII range
        if (*str < 32 || *str > 126) {
            str++;
            continue;
        }

        // Calculate character index (0-94 for ASCII 32-126)
        uint8_t char_index = (*str - 32);

        // Make sure we don't overflow the font array
        if (char_index > 94) {
            str++;
            continue;
        }

        // Calculate pointer to character data
        const uint8_t *char_ptr = &Font5x7[char_index * 5];

        // Draw the character (5 columns)
        for(int i = 0; i < 5; i++) {
            // Check if we're still within screen bounds
            if (col + i >= 128) break;

            uint32_t buf_idx = (page * 128) + col + i;
            if (buf_idx < 1024) {
                SSD1306_Buffer[buf_idx] = char_ptr[i];
            }
        }

        // Add 1px gap between characters
        col += 5;  // Move past the 5 columns we just wrote

        if (col < 128) {
            uint32_t gap_idx = (page * 128) + col;
            if (gap_idx < 1024) {
                SSD1306_Buffer[gap_idx] = 0x00;
            }
            col += 1;  // Add the gap column
        }

        str++;
    }
    }
