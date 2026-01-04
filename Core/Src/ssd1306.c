#include "ssd1306.h"
#include <string.h>

/* ================= Framebuffer ================= */
uint8_t ssd1306_buffer[SSD1306_WIDTH * SSD1306_PAGES];

/* ================= 6x8 Font ================= */
const uint8_t Font6x8[][6] = {
    /* ASCII 32-127 */
    {0,0,0,0,0,0}, // space
    {0x00,0x00,0x5F,0x00,0x00,0x00}, // !
    // ... fill rest as needed
};

/* ================= Low-level commands ================= */
static void ssd1306_send_command(uint8_t cmd)
{
    uint8_t buf[2] = {0x00, cmd}; // 0x00 = command
    i2c_dma_send(buf, 2);
    while(!i2c_dma_ready());
}

/* ================= Initialization ================= */
void ssd1306_init(void)
{
    i2c_dma_init();
    memset(ssd1306_buffer, 0x00, sizeof(ssd1306_buffer));

    ssd1306_send_command(0xAE); // display off
    ssd1306_send_command(0x20); // memory addressing mode
    ssd1306_send_command(0x00); // horizontal addressing
    ssd1306_send_command(0xB0); // page start 0
    ssd1306_send_command(0xC8); // COM scan dec
    ssd1306_send_command(0x00); // low column
    ssd1306_send_command(0x10); // high column
    ssd1306_send_command(0x40); // start line 0
    ssd1306_send_command(0x81); // contrast
    ssd1306_send_command(0x7F);
    ssd1306_send_command(0xA1); // seg remap
    ssd1306_send_command(0xA6); // normal display
    ssd1306_send_command(0xA8); // multiplex
    ssd1306_send_command(0x3F);
    ssd1306_send_command(0xA4); // display all on resume
    ssd1306_send_command(0xD3); // display offset
    ssd1306_send_command(0x00);
    ssd1306_send_command(0xD5); // display clock
    ssd1306_send_command(0xF0);
    ssd1306_send_command(0xD9); // precharge
    ssd1306_send_command(0x22);
    ssd1306_send_command(0xDA); // COM pins
    ssd1306_send_command(0x12);
    ssd1306_send_command(0xDB); // VCOM detect
    ssd1306_send_command(0x20);
    ssd1306_send_command(0x8D); // charge pump
    ssd1306_send_command(0x14);
    ssd1306_send_command(0xAF); // display on
}

#include "ssd1306.h"
#include "i2c_dma.h"
#include <string.h>
#include <stdio.h>

/* ================= Cursor ================= */
static uint8_t cursor_x = 0;
static uint8_t cursor_page = 0;

/* Set the current cursor position */
void ssd1306_set_cursor(uint8_t x, uint8_t page)
{
    if(x >= SSD1306_WIDTH) x = SSD1306_WIDTH - 1;
    if(page >= SSD1306_PAGES) page = SSD1306_PAGES - 1;

    cursor_x = x;
    cursor_page = page;
}

/* ================= Drawing ================= */

/* Draw a single 6x8 character at the current cursor */
void ssd1306_draw_char(char c)
{
    if(c < 32 || c > 127) c = '?';  // fallback for unsupported chars

    const uint8_t *p = Font6x8[c - 32];

    for(uint8_t i = 0; i < 6; i++)
    {
        uint16_t index = cursor_page * SSD1306_WIDTH + cursor_x;
        if(index < SSD1306_WIDTH * SSD1306_PAGES)
            ssd1306_buffer[index] = p[i];

        cursor_x++;
        if(cursor_x >= SSD1306_WIDTH)
        {
            cursor_x = 0;
            cursor_page++;
            if(cursor_page >= SSD1306_PAGES) cursor_page = SSD1306_PAGES - 1;
        }
    }
}

/* Draw a null-terminated string starting at current cursor */
void ssd1306_draw_string(const char *str)
{
    while(*str) ssd1306_draw_char(*str++);
}

/* Clear the framebuffer and reset cursor */
void ssd1306_clear(void)
{
    memset(ssd1306_buffer, 0, sizeof(ssd1306_buffer));
    cursor_x = 0;
    cursor_page = 0;
}

/* ================= Update via DMA ================= */
void ssd1306_update(void)
{
    uint8_t buf[SSD1306_WIDTH * SSD1306_PAGES + 1];
    buf[0] = 0x40; // Control byte for data
    memcpy(&buf[1], ssd1306_buffer, SSD1306_WIDTH * SSD1306_PAGES);
    i2c_dma_send(buf, SSD1306_WIDTH * SSD1306_PAGES + 1);
}

/* ================= High-level helpers ================= */

/* Show monitoring screen with pitch and roll */
void ssd1306_show_monitoring(int16_t pitch, int16_t roll)
{
    ssd1306_clear();
    ssd1306_set_cursor(0, 0);
    ssd1306_draw_string("MONITORING");

    char line[20];
    snprintf(line, sizeof(line), "P:%d R:%d", pitch, roll);
    ssd1306_set_cursor(0, 2);
    ssd1306_draw_string(line);

    ssd1306_update();
}

/* Show shock detected screen */
void ssd1306_show_shock_detected(void)
{
    ssd1306_clear();
    ssd1306_set_cursor(0, 3);
    ssd1306_draw_string("SHOCK DETECTED!");
    ssd1306_update();
}
