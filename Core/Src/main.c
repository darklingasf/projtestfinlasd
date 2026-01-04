#include "stm32f411xe.h"
#include "uart.h"
#include "spi.h"
#include "adxl345.h"
#include "exti.h"
#include "tim.h"
#include "log.h"
#include "ssd1306.h"
#include <math.h>
#include <stdio.h>
#include "log.h"

/* Flags from ISRs */
extern volatile uint8_t oled_update_flag;
extern volatile uint8_t adxl_int_flag;
volatile uint8_t oled_frozen = 0;
volatile uint8_t shock_detected = 0;

static void compute_angles(int16_t ax, int16_t ay, int16_t az,
                            float *pitch, float *roll)
{
    float fax = (float)ax;
    float fay = (float)ay;
    float faz = (float)az;

    *roll  = atan2f(fay, faz) * 57.2958f;
    *pitch = atan2f(-fax, sqrtf(fay*fay + faz*faz)) * 57.2958f;
}

int main(void)
{
    /* ---------- INIT ---------- */
    uart_init();
    uart_send_string("System ready\r\n");

    spi_gpio_init();
    spi1_config();
    adxl_init();
    adxl_enable_activity_interrupt(16);

    adxl_exti_init();
    tim2_init();
    tim2_start();

    SSD1306_Init();

    uart_send_string("Init complete\r\n");

    /* ---------- MAIN LOOP ---------- */
    while (1)
    {
        uart_process();

        /* Periodic sampling (TIM sets oled_update_flag) */
        if (oled_update_flag)
        {
            oled_update_flag = 0;

            int16_t ax, ay, az;
            float pitch, roll;
            char line[20];

            adxl_read_xyz(&ax, &ay, &az);

            /* ALWAYS log raw data */
            log_add(ax, ay, az);

            /* If shock happened → freeze OLED */
            if (shock_detected)
            {
                shock_detected = 0;
                oled_frozen = 1;

                SSD1306_Clear();
                SSD1306_PrintLabel(2, 0, "SHOCK");
                SSD1306_PrintLabel(4, 0, "DETECTED");
                SSD1306_Update();
            }

            /* Normal OLED update */
            if (!oled_frozen)
            {
                compute_angles(ax, ay, az, &pitch, &roll);

                SSD1306_Clear();

                SSD1306_PrintLabel(0, 0, "MONITORING");

                snprintf(line, sizeof(line), "PITCH: %.1f", pitch);
                SSD1306_PrintLabel(2, 0, line);

                snprintf(line, sizeof(line), "ROLL : %.1f", roll);
                SSD1306_PrintLabel(4, 0, line);

                SSD1306_Update();
            }
        }
    }
}
