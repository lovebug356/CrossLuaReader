#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "logging.h"
#include "hal_display.h"
#include "hal_gpio.h"
#include "Arduino.h"
#include "emulator_ui.h"

static SDL_Window *window = NULL;
static SDL_Renderer *sdl_renderer = NULL;
static SDL_Texture *texture = NULL;

// The default display setup is in portrait mode
#define DISP_WIDTH  480
#define DISP_HEIGHT 800
static uint8_t framebuffer[DISP_WIDTH * DISP_HEIGHT / 8];

void hal_display_set_x3(void) {}

bool hal_display_init(void) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG_ERR("EMULATOR-DISPLAY", "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    window = SDL_CreateWindow("CrossLuaReader Emulator",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              EMU_WINDOW_WIDTH, EMU_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        LOG_ERR("EMULATOR-DISPLAY", "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    sdl_renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STREAMING, DISP_WIDTH, DISP_HEIGHT);
    
    // Initial clear
    memset(framebuffer, 0xFF, sizeof(framebuffer));
    LOG_INF("EMULATOR-DISPLAY", "SDL2 Display initialized");

    emu_ui_init(sdl_renderer, texture);
    emu_ui_update();

    return true;
}

void hal_display_clear(uint8_t color) {
    memset(framebuffer, color, sizeof(framebuffer));
}

void hal_display_update_ui(void) {
    emu_ui_update();
}

void hal_display_refresh(refresh_mode_t mode) {
    uint32_t *pixels;
    int pitch;
    SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);

    for (int y = 0; y < DISP_HEIGHT; y++) {
        for (int x = 0; x < DISP_WIDTH; x++) {
            int byte_idx = (x * (DISP_HEIGHT / 8)) + (y / 8);
            uint8_t bit = 7 - (y % 8);
            bool white = (framebuffer[byte_idx] >> bit) & 1;
            uint32_t color = white ? 0xFFFFFF : 0x000000;

            pixels[y * (pitch / 4) + DISP_WIDTH - x - 1] = color;
        }
    }

    SDL_UnlockTexture(texture);
    emu_ui_update();
}

void hal_display_deep_sleep(void) {
    LOG_INF("EMULATOR-DISPLAY", "Display deep sleep (mock)");
}

void hal_display_screenshot(const char *filename) {
    if (!texture || !sdl_renderer) {
        LOG_ERR("EMULATOR-DISPLAY", "Cannot take screenshot: display not initialized");
        return;
    }

    // Lock the texture to access pixel data
    uint32_t *pixels;
    int pitch;
    SDL_LockTexture(texture, NULL, (void**)&pixels, &pitch);

    // Create a grayscale (8-bit indexed) surface
    SDL_Surface *surface = SDL_CreateRGBSurface(0, DISP_WIDTH, DISP_HEIGHT, 8, 0, 0, 0, 0);
    if (!surface) {
        SDL_UnlockTexture(texture);
        LOG_ERR("EMULATOR-DISPLAY", "Failed to create grayscale surface");
        return;
    }

    // Create and set a linear grayscale palette (0-255)
    SDL_Palette *palette = SDL_AllocPalette(256);
    if (!palette) {
        SDL_UnlockTexture(texture);
        SDL_FreeSurface(surface);
        LOG_ERR("EMULATOR-DISPLAY", "Failed to create palette");
        return;
    }

    for (int i = 0; i < 256; i++) {
        palette->colors[i] = (SDL_Color){i, i, i, 255};
    }
    SDL_SetSurfacePalette(surface, palette);

    // Convert RGB pixels to grayscale (8-bit)
    uint8_t *surface_pixels = (uint8_t *)surface->pixels;
    for (int y = 0; y < DISP_HEIGHT; y++) {
        for (int x = 0; x < DISP_WIDTH; x++) {
            uint32_t rgb = pixels[y * (pitch / 4) + x];
            
            // Extract RGB components
            uint8_t r = (rgb >> 24) & 0xFF;
            uint8_t g = (rgb >> 16) & 0xFF;
            uint8_t b = (rgb >> 8) & 0xFF;
            
            // Convert to grayscale using standard luminance formula
            uint8_t gray = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
            
            surface_pixels[y * surface->pitch + x] = gray;
        }
    }

    SDL_UnlockTexture(texture);

    // Save as BMP
    int result = SDL_SaveBMP(surface, filename);
    if (result != 0) {
        LOG_ERR("EMULATOR-DISPLAY", "Failed to save screenshot: %s", SDL_GetError());
    } else {
        LOG_INF("EMULATOR-DISPLAY", "Screenshot saved to: %s", filename);
    }

    SDL_FreePalette(palette);
    SDL_FreeSurface(surface);
}

uint8_t *hal_display_get_framebuffer(void) {
    return framebuffer;
}


int hal_display_width(void) { return DISP_HEIGHT; }
int hal_display_height(void) { return DISP_WIDTH; }
int hal_display_width_bytes(void) { return DISP_HEIGHT / 8; }
uint32_t hal_display_buffer_size(void) { return sizeof(framebuffer); }
void hal_display_request_resync(uint8_t settle_passes) {}

// Cleanup function
void hal_display_quit(void) {
    emu_ui_quit();
    if (texture) SDL_DestroyTexture(texture);
    if (sdl_renderer) SDL_DestroyRenderer(sdl_renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}
