#ifndef LOG_H
#define LOG_H

#include <stdint.h>
#include <stdbool.h>

/* ===== Configuration ===== */
#define LOG_BUF_SIZE 100   // Required by spec

/* ===== Data Type ===== */
typedef struct {
    int16_t ax;
    int16_t ay;
    int16_t az;
} log_sample_t;

/* ===== API ===== */

/* Reset buffer and prepare for new capture */
void log_start_capture(void);

/* Add one sample (called at 20 Hz) */
void log_add_sample(int16_t ax, int16_t ay, int16_t az);

/* Status queries */
bool log_is_active(void);
bool log_is_complete(void);

/* Access for UART */
const log_sample_t* log_get_buffer(void);
uint16_t log_get_count(void);

#endif /* LOG_H */
