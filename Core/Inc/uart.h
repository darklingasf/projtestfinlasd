#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

/* Initialize UART2: 115200, 8N1 */
void uart_init(void);

/* Send data */
void uart_send_char(char c);
void uart_send_string(const char *str);

/* Called in main loop to process incoming commands */
void uart_process(void);

/* Command hooks - implement in main.c */
void uart_cmd_set_thresh(uint16_t value);
void uart_cmd_get_log(void);

#endif /* UART_H */
