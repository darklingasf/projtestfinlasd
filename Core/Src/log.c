#include "log.h"

/* ===== Internal State ===== */

static log_sample_t log_buffer[LOG_BUF_SIZE];

static volatile uint16_t write_index = 0;
static volatile bool capture_active = false;
static volatile bool capture_complete = false;

/* ===== API Implementation ===== */

void log_start_capture(void)
{
    write_index = 0;
    capture_complete = false;
    capture_active = true;
}

void log_add_sample(int16_t ax, int16_t ay, int16_t az)
{
    if (!capture_active)
        return;

    log_buffer[write_index].ax = ax;
    log_buffer[write_index].ay = ay;
    log_buffer[write_index].az = az;

    write_index++;

    if (write_index >= LOG_BUF_SIZE)
    {
        capture_active = false;
        capture_complete = true;
    }
}

bool log_is_active(void)
{
    return capture_active;
}

bool log_is_complete(void)
{
    return capture_complete;
}

const log_sample_t* log_get_buffer(void)
{
    return log_buffer;
}

uint16_t log_get_count(void)
{
    return write_index;
}
