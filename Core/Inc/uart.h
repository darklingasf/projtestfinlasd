#ifndef UART_H
#define UART_H

#include <stdint.h>

/* ================= PUBLIC API ================= */
void uart_init(void);
void uart_send_char(char c);
void uart_send_string(const char *str);
void uart_process(void);         // Call in main loop

/* ========== Weak hooks for custom behavior ========= */
void uart_cmd_set_thresh(uint16_t value);
void uart_cmd_get_log(void);

#endif /* UART_H */
