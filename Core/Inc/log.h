#ifndef LOG_H_
#define LOG_H_

#include <stdint.h>
#include <stdbool.h>

#define LOG_BUF_SIZE 100

typedef struct
{
    int16_t ax;
    int16_t ay;
    int16_t az;
} log_sample_t;

/* API */
void log_start_capture(void);
void log_add(int16_t ax, int16_t ay, int16_t az);
bool log_is_active(void);
bool log_is_complete(void);
const log_sample_t* log_get_buffer(void);
uint16_t log_get_count(void);
void log_reset(void);

#endif /* LOG_H_ */
