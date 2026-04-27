#include <SDL2/SDL.h>
#include <pthread.h>

#include "hal_gpio.h"
#include "logging.h"
#include "Arduino.h"
#include "hal_display.h"
#include "emulator_ui.h"

static device_type_t detected_device = DEVICE_X4;
static bool initialized = false;
static bool btn_state[BTN_COUNT] = {false};
static bool btn_was_pressed[BTN_COUNT] = {false};
static bool btn_was_released[BTN_COUNT] = {false};
static uint32_t btn_held_start[BTN_COUNT] = {0};
static int mouse_pressed_btn = -1;
static pthread_mutex_t gpio_mutex = PTHREAD_MUTEX_INITIALIZER;

static void set_button_state(int btn, bool pressed) {
    if (btn < 0 || btn >= BTN_COUNT) return;
    
    pthread_mutex_lock(&gpio_mutex);
    if (pressed) {
        if (!btn_state[btn]) {
            btn_was_pressed[btn] = true;
            btn_held_start[btn] = millis();
        }
        btn_state[btn] = true;
    } else {
        if (btn_state[btn]) {
            btn_was_released[btn] = true;
            btn_held_start[btn] = 0;
        }
        btn_state[btn] = false;
    }
    pthread_mutex_unlock(&gpio_mutex);
}

bool hal_gpio_init(void) {
    if (initialized) return true;

    LOG_INF("EMULATOR-GPIO", "Initializing input manager");
    pthread_mutex_lock(&gpio_mutex);
    for (int i = 0; i < BTN_COUNT; i++) {
        btn_state[i] = false;
        btn_was_pressed[i] = false;
        btn_was_released[i] = false;
        btn_held_start[i] = 0;
    }
    mouse_pressed_btn = -1;
    pthread_mutex_unlock(&gpio_mutex);

    initialized = true;
    return true;
}

void hal_gpio_poll(void) {
    // This is called by the background timer thread.
    // We MUST NOT call SDL_PollEvent here because it's not thread-safe.
    // Instead, we only clear the "was" flags.
    pthread_mutex_lock(&gpio_mutex);
    for (int i = 0; i < BTN_COUNT; i++) {
        btn_was_pressed[i] = false;
        btn_was_released[i] = false;
    }
    pthread_mutex_unlock(&gpio_mutex);
}

/** 
 * Actual SDL event polling. MUST be called from the main thread.
 * We'll call this from hal_display_update_ui or a similar main-thread hook.
 */
void hal_gpio_emulator_poll_events(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            LOG_INF("EMULATOR-GPIO", "Quit event received, exiting\n");
            exit(0);
        } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            bool pressed = (e.type == SDL_KEYDOWN);
            switch (e.key.keysym.sym) {
                case SDLK_UP:     set_button_state(BTN_UP, pressed); break;
                case SDLK_DOWN:   set_button_state(BTN_DOWN, pressed); break;
                case SDLK_LEFT:   set_button_state(BTN_LEFT, pressed); break;
                case SDLK_RIGHT:  set_button_state(BTN_RIGHT, pressed); break;
                case SDLK_RETURN: set_button_state(BTN_CONFIRM, pressed); break;
                case SDLK_ESCAPE: set_button_state(BTN_BACK, pressed); break;
                case SDLK_p:      set_button_state(BTN_POWER, pressed); break;
                case SDLK_s:      if (pressed) hal_display_screenshot("screenshot.bmp"); break;
                default: break;
            }
        } else if (e.type == SDL_MOUSEBUTTONDOWN) {
            int x = e.button.x;
            int y = e.button.y;
            for (int i = 0; i < BTN_COUNT; i++) {
                const emu_ui_button_t *btn = &emu_ui_buttons[i];
                if (x >= btn->x && x <= btn->x + btn->w &&
                    y >= btn->y && y <= btn->y + btn->h) {
                    mouse_pressed_btn = btn->btn_index;
                    set_button_state(mouse_pressed_btn, true);
                    break;
                }
            }
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            if (mouse_pressed_btn != -1) {
                set_button_state(mouse_pressed_btn, false);
                mouse_pressed_btn = -1;
            }
        }
    }
}

bool hal_gpio_is_pressed(uint8_t button) {
    if (button >= BTN_COUNT) return false;
    pthread_mutex_lock(&gpio_mutex);
    bool state = btn_state[button];
    pthread_mutex_unlock(&gpio_mutex);
    return state;
}

bool hal_gpio_was_pressed(uint8_t button) {
   if (button >= BTN_COUNT) return false;
    pthread_mutex_lock(&gpio_mutex);
    bool state = btn_was_pressed[button];
    pthread_mutex_unlock(&gpio_mutex);
    return state;
}

bool hal_gpio_was_any_pressed(void) {
    pthread_mutex_lock(&gpio_mutex);
    for (int i = 0; i < BTN_COUNT; i++) {
        if (btn_was_pressed[i]) {
            pthread_mutex_unlock(&gpio_mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&gpio_mutex);
    return false;
}

bool hal_gpio_was_released(uint8_t button) {
    if (button >= BTN_COUNT) return false;
    pthread_mutex_lock(&gpio_mutex);
    bool state = btn_was_released[button];
    pthread_mutex_unlock(&gpio_mutex);
    return state;
}

bool hal_gpio_was_any_released(void) {
    pthread_mutex_lock(&gpio_mutex);
    for (int i = 0; i < BTN_COUNT; i++) {
        if (btn_was_released[i]) {
            pthread_mutex_unlock(&gpio_mutex);
            return true;
        }
    }
    pthread_mutex_unlock(&gpio_mutex);
    return false;
}

unsigned long hal_gpio_get_held_time(void) {
    unsigned long max_held = 0;
    uint32_t now = millis();
    pthread_mutex_lock(&gpio_mutex);
    for (int i = 0; i < BTN_COUNT; i++) {
        if (btn_state[i] && btn_held_start[i] > 0) {
            unsigned long held = now - btn_held_start[i];
            if (held > max_held) max_held = held;
        }
    }
    pthread_mutex_unlock(&gpio_mutex);
    return max_held;
}

device_type_t hal_gpio_get_device_type(void) {
    return detected_device;
}

bool hal_gpio_is_x3(void) {
    return detected_device == DEVICE_X3;
}

bool hal_gpio_is_x4(void) {
    return detected_device == DEVICE_X4;
}

void hal_gpio_start_deep_sleep(void) {
    LOG_INF("EMULATOR-GPIO", "Entering deep sleep (exiting)\n");
    exit(0);

}
