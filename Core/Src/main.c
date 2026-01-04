#include "stm32f4xx.h"
#include "uart.h"
#include "spi.h"
#include "adxl345.h"
#include "exti.h"
#include "tim.h"
#include "log.h"
#include "ssd1306.h"
#include <stdint.h>
#include <stdio.h>

/* --- Flags --- */
/* --- UART Hooks --- */
void uart_cmd_set_thresh(uint16_t val) {
    adxl_enable_activity_interrupt(val);
    uart_send_string("Threshold updated\r\n");
}

void uart_cmd_get_log(void) {
    log_start_capture();   // reset log capture
    ssd1306_show_monitoring(0, 0); // reset display
    uart_send_string("LOG RESET & MONITORING\r\n");
}

/* ================= Integer atan2 for STM32 ================= */
int16_t atan2_int(int16_t y, int16_t x) {
    int32_t abs_y = (y < 0) ? -y : y;
    int32_t abs_x = (x < 0) ? -x : x;
    int16_t angle;

    if (abs_x == 0 && abs_y == 0) return 0;

    if (abs_x >= abs_y) angle = (45 * abs_y) / abs_x;
    else angle = 90 - (45 * abs_x) / abs_y;

    if (x < 0) {
        if (y >= 0) return 180 - angle;
        else return -180 + angle;
    }
    return (y >= 0) ? angle : -angle;
}

int main(void) {
    /* --- Init Peripherals --- */
    uart_init();
    spi_gpio_init();
    spi1_config();
    adxl_init();
    adxl_enable_activity_interrupt(16);
    adxl_exti_init();
    tim2_init();          // 20 Hz sampling timer
    ssd1306_init();
    ssd1306_show_monitoring(0, 0);

    adxl_write(0x2D, 0x08); // Measure mode
    uart_send_string("System Ready\r\n");

    int16_t f_roll = 0, f_pitch = 0;

    while(1) {
        /* --- Process UART --- */
        uart_process();

        /* --- ADXL Interrupt --- */
        if(adxl_int_flag) {
            adxl_int_flag = 0;
            log_start_capture();

            ssd1306_show_shock_detected();
            uart_send_string("Shock detected!\r\n");
        }

        /* --- Update pitch/roll display if monitoring --- */
        /* --- Update pitch/roll display if monitoring --- */
        if(log_is_active()) {
            int16_t ax, ay, az;
            adxl_read_xyz(&ax, &ay, &az);  // <-- use the high-level API

            // Calculate angles (integer atan2)
            int16_t roll  = atan2_int(ay, az);
            int16_t pitch = atan2_int(-ax, az);

            // Simple low-pass filter
            f_roll  = (f_roll * 3 + roll) >> 2;
            f_pitch = (f_pitch * 3 + pitch) >> 2;

            ssd1306_show_monitoring(f_pitch, f_roll);
        }


        /* --- Print log if capture complete --- */
        if(log_is_complete()) {
            uart_send_string("LOG COMPLETE\r\n");
            uint16_t count = log_get_count();
            const log_sample_t* buf = log_get_buffer();
            for(uint16_t i = 0; i < count; i++) {
                char line[64];
                snprintf(line, sizeof(line), "%d,%d,%d\r\n",
                         buf[i].ax, buf[i].ay, buf[i].az);
                uart_send_string(line);
            }
        }
    }
}
