/* log.c */
#include "log.h"
#include <stdint.h>
#include <stdbool.h>

static log_sample_t log_buffer[LOG_BUF_SIZE];
static volatile uint16_t write_index = 0;
static volatile bool capture_active = false;
static volatile bool capture_complete = false;

void log_start_capture(void)
{
    write_index = 0;
    capture_active = true;
    capture_complete = false;
}

void log_add(int16_t ax, int16_t ay, int16_t az)
{
    if (!capture_active) return;

    log_buffer[write_index].ax = ax;
    log_buffer[write_index].ay = ay;
    log_buffer[write_index].az = az;

    write_index++;
    if (write_index >= LOG_BUF_SIZE)
    {
        capture_active = false;
        capture_complete = true;   // <- set complete
    }
}

bool log_is_active(void) { return capture_active; }
bool log_is_complete(void) { return capture_complete; }

const log_sample_t* log_get_buffer(void) { return log_buffer; }
uint16_t log_get_count(void) { return write_index; }

void log_reset(void)
{
    write_index = 0;
    capture_active = false;
    capture_complete = false;
}
