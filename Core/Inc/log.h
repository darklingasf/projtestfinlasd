#ifndef LOG_H
#define LOG_H

#include <stdint.h>

typedef struct {
    int16_t x, y, z;
} LogEntry;

void LOG_Write(int16_t x, int16_t y, int16_t z);
void LOG_StreamUART(void);

#endif
