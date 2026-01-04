#include "stm32f4xx.h"
#include <stdio.h>
#include <math.h>

// Driver Includes
#include "adxl345.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"
#include "i2c_dma.h"
#include "uart.h"
#include "tim.h"

// --- Global Flags & Buffers ---
volatile uint8_t shock_detected = 0;
volatile uint32_t shock_counter = 0;
char display_buffer[32];

// --- Function Prototypes ---
void Process_Orientation(void);
void System_Init(void);

int main(void) {
    System_Init();

    UART2_SendString("System Booted: Monitoring Active\r\n");

    while (1) {
        // 1. Preparation
        SSD1306_Clear();

        // 2. Header
        SSD1306_PrintLabel(0, 30, "--- MONITORING ---");

        // 3. Sensor & Display Logic
        Process_Orientation();

        // 4. Shock Alert Logic (Triggered by EXTI)
        if (shock_detected) {
            SSD1306_PrintLabel(6, 15, "!! SHOCK DETECTED !!");

            // Hold message for ~3 seconds based on loop speed
            if (shock_counter > 0) {
                shock_counter--;
            } else {
                shock_detected = 0;
            }
        }

        // 5. Update Screen via I2C DMA
        SSD1306_Update();

        // 6. Loop Timing (Approx 20Hz refresh)
        delay_ms(50);
    }
}

/**
 * @brief Initializes all user-defined modules
 */
void System_Init(void) {
    // Basic MCU / Timer Init
    TIM2_Init();
    UART2_Init();

    // I2C & OLED Init
    I2C1_DMA_Init();
    SSD1306_Init();

    // Accelerometer Init (Including EXTI config inside)
    ADXL345_Init();
}

/**
 * @brief Reads ADXL345, calculates angles, and updates display buffer
 */
void Process_Orientation(void) {
    int16_t x, y, z;
    float pitch, roll;

    // Use your ADXL driver to get raw data
    ADXL345_ReadAccel(&x, &y, &z);

    // Standard Pitch & Roll calculation
    // Pitch: Rotation around X-axis
    // Roll:  Rotation around Y-axis
    pitch = atan2f((float)y, (float)z) * 57.2958f;
    roll  = atan2f(-(float)x, sqrtf((float)y * y + (float)z * z)) * 57.2958f;

    // Print to OLED Buffer
    sprintf(display_buffer, "PITCH: %+06.1f", pitch);
    SSD1306_PrintLabel(2, 10, display_buffer);

    sprintf(display_buffer, "ROLL:  %+06.1f", roll);
    SSD1306_PrintLabel(4, 10, display_buffer);
}

/**
 * @brief EXTI Interrupt Handler for ADXL345 INT1
 * Ensure this matches the pin you connected (e.g., EXTI0 for Pin 0)
 */
void EXTI0_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PR0) {
        EXTI->PR |= EXTI_PR_PR0; // Clear Pending Bit

        shock_detected = 1;
        shock_counter = 60; // 60 iterations * 50ms = 3 seconds

        UART2_SendString("ALERT: Physical Shock Sensed!\r\n");
    }
}
