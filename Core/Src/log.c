#include "log.h"
#include "uart.h"
#include <stdio.h>

// Buffer to hold 100 samples (Requirement Mode 2)
static LogEntry log_buffer[100];
static uint16_t current_entry = 0;

/**
 * @brief Saves a single 3-axis sample into the RAM buffer
 */
void LOG_Write(int16_t x, int16_t y, int16_t z) {
    if (current_entry < 100) {
        log_buffer[current_entry].x = x;
        log_buffer[current_entry].y = y;
        log_buffer[current_entry].z = z;
        current_entry++;
    }
}

/**
 * @brief Streams all stored data points via UART to the PC
 * Requirement P4: UART Command Interface
 */
void LOG_StreamUART(void) {
    char tx_msg[64];

    UART2_SendString("\r\n--- START OF LOG DATA ---\r\n");

    for (uint16_t i = 0; i < current_entry; i++) {
        // Format: "Index, X, Y, Z"
        sprintf(tx_msg, "[%d] X:%d Y:%d Z:%d\r\n", i,
                log_buffer[i].x, log_buffer[i].y, log_buffer[i].z);
        UART2_SendString(tx_msg);
    }

    UART2_SendString("--- END OF LOG ---\r\n");

    // Reset index for the next shock event
    current_entry = 0;
}
