#include "sleep_screen.h"
#include "Arduino.h"
#include "logging.h"

void sleep_screen_set_mode(sleep_mode_t mode) {
    LOG_INF("EMULATOR-SLEEP", "Mode set to %d", (int)mode);
}

/** Set the wallpaper filename for SINGLE mode (e.g., "sunset.bmp"). */
void sleep_screen_set_wallpaper(const char *filename) {
    LOG_INF("EMULATOR-SLEEP", "Wallpaper set to %s", filename ? filename : "NULL");
}

/**
 * Register a Lua callback to be called after the base sleep screen renders
 * but before the display refresh. The callback can draw text, shapes, etc.
 * using normal display.* API calls (they write to the framebuffer).
 *
 * @param L   Lua state that owns the callback
 * @param ref Lua registry reference to the callback function (from luaL_ref)
 */
void sleep_screen_set_hook(lua_State *L, int ref) {
    LOG_INF("EMULATOR-SLEEP", "Sleep hook set");
}

/**
 * Clear the sleep hook. Call when the plugin that set it exits.
 */
void sleep_screen_clear_hook(void) {
    LOG_INF("EMULATOR-SLEEP", "Sleep hook cleared");
}

/**
 * Render the sleep screen to the framebuffer and refresh the display.
 * Called from hal_power_enter_sleep() before entering deep sleep.
 * If a Lua hook is registered, it is called after the base screen renders.
 */
void sleep_screen_render(void) {
    LOG_INF("EMULATOR-SLEEP", "Rendering sleep screen");
}
