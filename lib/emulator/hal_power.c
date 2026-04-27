#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "Arduino.h"
#include "logging.h"

static uint16_t battery_percent = 100;
static uint32_t last_bat_update = 0;

/**
 * Initialize power management and battery monitor.
 * Must be called after hal_gpio_init() (needs device type for X3/X4).
 *
 * @return true on success
 */
bool hal_power_init(void) {
    LOG_INF("EMULATOR-PWR", "Initializing power management");
    last_bat_update = millis();
    return true;
}

/** @return Battery percentage (0-100). Cached, polled every 30 seconds. */
uint16_t hal_power_battery_percent(void) {
    // LOG_DBG("EMULATOR-PWR", "Reading battery percentage");

    uint32_t now = millis();
    if (now - last_bat_update > 60000) { // Every minute
        if (battery_percent > 1) battery_percent--;
        last_bat_update = now;
    }
    return battery_percent;
}

/** @return Battery voltage in millivolts. */
uint16_t hal_power_battery_millivolts(void) {
    // LOG_DBG("EMULATOR-PWR", "Reading battery voltage");

    return 3500 + (battery_percent * 7); // Mock 3.5V - 4.2V
}

/**
 * Check if device should auto-sleep due to inactivity.
 * Call once per loop iteration.
 */
void hal_power_check_sleep(void) {
    // LOG_INF("EMULATOR-PWR", "Checking sleep conditions");
}

/**
 * Set the auto-sleep timeout.
 *
 * @param minutes Minutes of inactivity before sleep (1-30). 0 = disable.
 */
void hal_power_set_sleep_timeout(uint32_t minutes) {
    LOG_INF("EMULATOR-PWR", "Setting sleep timeout");
}

/**
 * Suppress or restore auto-sleep.
 * Use when WiFi server is active, USB connected, or downloading.
 *
 * @param suppress true to prevent sleep, false to restore normal behavior
 */
void hal_power_suppress_sleep(bool suppress) {
    LOG_INF("EMULATOR-PWR", "Sleep %s", suppress ? "suppressed" : "restored");
}

/** @return true if USB is connected (suppresses auto-sleep). */
bool hal_power_is_usb_connected(void) {
    LOG_INF("EMULATOR-PWR", "Checking USB connection");
    return false;
}

/** Enter deep sleep immediately. Does not return. */
void hal_power_enter_sleep(void) {
    LOG_INF("EMULATOR-PWR", "Entering deep sleep");
}