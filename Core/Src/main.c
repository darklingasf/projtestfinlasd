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

void debug_keep_alive_in_sleep(void)
{
    // DBGMCU_CR register controls debug behavior in low-power modes
    // Bit 0 = DBG_SLEEP: Keep debug active during Sleep mode
    // Bit 1 = DBG_STOP : Keep debug active during Stop mode (optional)
    // Bit 2 = DBG_STANDBY: Keep debug active during Standby (optional)
    DBGMCU->CR |= DBGMCU_CR_DBG_SLEEP;  // Always keep debug in Sleep

    // Optional: keep debug in Stop and Standby if you use them
    // DBGMCU->CR |= DBGMCU_CR_DBG_STOP;
    // DBGMCU->CR |= DBGMCU_CR_DBG_STANDBY;
}

/* -------- ISR FLAGS -------- */
extern volatile uint8_t oled_update_flag;
extern volatile uint8_t uart_get_log_received;

/* -------- SYSTEM STATE -------- */
volatile uint8_t shock_detected = 0;
volatile uint8_t oled_frozen       = 0;
volatile uint8_t capture_armed     = 1;

/* -------- ANGLE MATH -------- */
static void compute_angles(int16_t ax, int16_t ay, int16_t az,
                           float *pitch, float *roll)
{
    float fax = (float)ax;
    float fay = (float)ay;
    float faz = (float)az;

    *roll  = atan2f(fay, faz) * 57.2958f;
    *pitch = atan2f(-fax, sqrtf(fay*fay + faz*faz)) * 57.2958f;
}

/* -------- OLED DISPLAY -------- */
void oled_show_monitoring(void)
{
    SSD1306_Clear();
    SSD1306_PrintLabel(0, 0, "MONITORING");
    SSD1306_Update();
}

static void oled_show_shock(void)
{
    SSD1306_Clear();
    SSD1306_PrintLabel(2, 0, "SHOCK");
    SSD1306_PrintLabel(4, 0, "DETECTED");
    SSD1306_Update();
}

void boot_delay_ms(uint32_t ms) {
    // Configure SysTick for 1ms interrupts
    SysTick->LOAD = 16000 - 1;  // 16MHz / 1000 = 16000 (for 1ms if running at 16MHz)
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_CLKSOURCE_Msk;

    for(uint32_t i = 0; i < ms; i++) {
        // Wait until COUNTFLAG is set
        while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    }

    SysTick->CTRL = 0;  // Disable SysTick
}

/* -------- MAIN LOOP -------- */
int main(void)
{

    __disable_irq();  // Start with interrupts globally disabled

    // All your initialization...

    // Clear ALL pending interrupts
    for(int i = 0; i < 8; i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;  // Disable all interrupts
        NVIC->ICPR[i] = 0xFFFFFFFF;  // Clear all pending
    }

    // 1-second delay
    for(volatile uint32_t i = 0; i < 16000000; i++);

    // Now configure and enable your specific interrupt
    __enable_irq();
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
    oled_show_monitoring();

    uart_send_string("Init complete\r\n");

    while (1)
    {
        uart_process();

        /* ---------- GET_LOG ---------- */
        if (uart_get_log_received)
        {
            uart_get_log_received = 0;

            // Dump log buffer over UART
            const log_sample_t* buf = log_get_buffer();
            uint16_t count = log_get_count();
            for (uint16_t i = 0; i < count; i++)
            {
                char msg[50];
                snprintf(msg, sizeof(msg), "%d,%d,%d\r\n",
                         buf[i].ax, buf[i].ay, buf[i].az);
                uart_send_string(msg);
            }

            uart_send_string("END_LOG\r\n");

            // Reset log buffer and rearm EXTI
            log_reset();
            capture_armed = 1;
            EXTI->IMR |= (1U << 8);

            // Unfreeze OLED and show monitoring screen
            oled_frozen = 0;
            oled_show_monitoring();

            uart_send_string("LOG CLEARED, EXTI REARMED\r\n");
        }

        /* ---------- PERIODIC SAMPLE ---------- */
        if (!oled_update_flag)
            continue;
        oled_update_flag = 0;

        int16_t ax, ay, az;
        float pitch, roll;
        adxl_read_xyz(&ax, &ay, &az);

        /* ---------- ACTIVE LOGGING ---------- */
        if (log_is_active())
        {
            log_add(ax, ay, az);

            if (log_is_complete())
                uart_send_string("LOG FINISHED\r\n");

            continue; // OLED frozen while logging
        }

        /* ---------- SHOCK DETECTED ---------- */
        if (shock_detected && capture_armed)
        {
            shock_detected = 0;
            capture_armed  = 0;
            oled_frozen    = 1;

            log_start_capture();
            uart_send_string("LOG STARTED\r\n");

            // Disable EXTI until GET_LOG
            EXTI->IMR &= ~(1U << 8);

            oled_show_shock();  // <-- now OLED will show shock detected
            continue;
        }

        /* ---------- NORMAL MONITORING MODE ---------- */
        if (!oled_frozen)
        {
            compute_angles(ax, ay, az, &pitch, &roll);

            SSD1306_Clear();
            SSD1306_PrintLabel(0, 0, "MONITORING");

            char line[32];
            snprintf(line, sizeof(line), "PITCH: %5.1f degrees", pitch);
            SSD1306_PrintLabel(2, 0, line);

            snprintf(line, sizeof(line), "ROLL : %5.1f degrees", roll);
            SSD1306_PrintLabel(4, 0, line);

            SSD1306_Update();
        }
        __WFI();
    }
}
