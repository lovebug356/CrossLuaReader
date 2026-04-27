#include "Arduino.h"
#include "freertos/task.h"
#include <chrono>
#include <thread>

MockSerial Serial;

unsigned long millis(void) {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void vTaskDelay(uint32_t ticks) {
    delay(ticks * portTICK_PERIOD_MS);
}

extern "C" {
    uint32_t uzlib_adler32(const void *data, unsigned int length, uint32_t prev_sum) { return 0; }
    uint32_t uzlib_crc32(const void *data, unsigned int length, uint32_t crc) { return 0; }
}
