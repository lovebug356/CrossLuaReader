#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include "freertos/task.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t byte;
typedef bool boolean;

unsigned long millis(void);
void delay(unsigned long ms);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
// Serial mock for C++ files
class MockSerial {
public:
    void begin(unsigned long baud) {}
    operator bool() { return true; }
    void print(const char* s) { printf("%s", s); }
    void println(const char* s) { printf("%s\n", s); }
    void printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
};

extern MockSerial Serial;
#endif
