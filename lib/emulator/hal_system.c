#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "logging.h"
#include "hal_system.h"

#ifndef CROSSLUA_READER_VERSION
#define CROSSLUA_READER_VERSION "0.0.0-emulator"
#endif

/* Boot time reference */
static struct timespec boot_time = {0, 0};

/* Simulated heap (matches ESP32-C3 constraint of ~380KB) */
#define SIMULATED_HEAP_SIZE 389120  /* 380KB in bytes */
static uint32_t heap_peak_usage = 0;        /* Peak heap usage so far */
static uint32_t min_free_heap = SIMULATED_HEAP_SIZE;  /* Minimum free seen */

/**
 * Initialize system-level hardware (watchdog, serial).
 * Must be called first in the boot sequence.
 *
 * @return true on success
 */
bool hal_system_init(void) {
    if (clock_gettime(CLOCK_MONOTONIC, &boot_time) != 0) {
        LOG_ERR("EMULATOR-SYSTEM", "Failed to get boot time");
        return false;
    }
    /* Reset heap tracking */
    heap_peak_usage = 0;
    min_free_heap = SIMULATED_HEAP_SIZE;
    return true;
}

/** Restart the device immediately. Does not return. */
void hal_system_restart(void) {
    LOG_INF("EMULATOR-SYSTEM", "Restarting device (exiting)\n");

    exit(0);
}

/** @return Current free heap in bytes. */
uint32_t hal_system_free_heap(void) {
    /* Simulate heap usage: assume ~100KB is currently in use */
    uint32_t simulated_usage = 102400;  /* ~100KB simulated allocation */
    uint32_t current_free = SIMULATED_HEAP_SIZE - simulated_usage;
    
    /* Update minimum if this is lower than previously seen */
    if (current_free < min_free_heap) {
        min_free_heap = current_free;
    }
    
    return current_free;
}

/** @return Total heap size in bytes. */
uint32_t hal_system_total_heap(void) {
    return SIMULATED_HEAP_SIZE;
}

/** @return Minimum free heap since boot (high-water mark). */
uint32_t hal_system_min_free_heap(void) {
    return min_free_heap;
}

/** @return Milliseconds since boot. */
uint32_t hal_system_uptime_ms(void) {
    struct timespec current_time;
    if (clock_gettime(CLOCK_MONOTONIC, &current_time) != 0) {
        LOG_ERR("EMULATOR-SYSTEM", "Failed to get current time");
        return 0;
    }

    /* Calculate elapsed time in milliseconds */
    uint32_t seconds_diff = (uint32_t)(current_time.tv_sec - boot_time.tv_sec);
    int32_t nanos_diff = current_time.tv_nsec - boot_time.tv_nsec;

    /* Handle negative nanosecond difference */
    if (nanos_diff < 0) {
        seconds_diff--;
        nanos_diff += 1000000000;
    }

    uint32_t millis = (seconds_diff * 1000) + (nanos_diff / 1000000);
    return millis;
}

/** @return Firmware version string. */
const char *hal_system_version(void) {
    return CROSSLUA_READER_VERSION;
}
