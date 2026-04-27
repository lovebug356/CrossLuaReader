#include "emulator_ui.h"
#include "hal_gpio.h"
#include <SDL2/SDL.h>

static SDL_Renderer *emu_renderer = NULL;
static SDL_Texture *emu_screen_texture = NULL;
static SDL_Texture *emu_logo_texture = NULL;
static int emu_logo_w = 0;
static int emu_logo_h = 0;

void emu_ui_init(SDL_Renderer *renderer, SDL_Texture *texture) {
    emu_renderer = renderer;
    emu_screen_texture = texture;

    // Load logo
    SDL_Surface *logo_surface = SDL_LoadBMP("lib/emulator/logo.bmp");
    if (logo_surface) {
        emu_logo_texture = SDL_CreateTextureFromSurface(renderer, logo_surface);
        emu_logo_w = logo_surface->w;
        emu_logo_h = logo_surface->h;
        SDL_FreeSurface(logo_surface);
    }
}

static void draw_logo(void) {
    if (emu_logo_texture) {
        int lx = EMU_SCREEN_X + (EMU_SCREEN_W / 2) - (emu_logo_w / 2);
        int ly = 30;
        SDL_Rect dest = { lx, ly, emu_logo_w, emu_logo_h };
        SDL_RenderCopy(emu_renderer, emu_logo_texture, NULL, &dest);
    }
}

void emu_ui_update(void) {
    if (!emu_renderer || !emu_screen_texture) return;

    // Poll SDL events on the main thread
    hal_gpio_emulator_poll_events();

    // 1. Draw device background (bezel)
    SDL_SetRenderDrawColor(emu_renderer, 45, 45, 45, 255); 
    SDL_RenderClear(emu_renderer);

    // 2. Draw Logo
    draw_logo();

    // 3. Draw screen border
    SDL_SetRenderDrawColor(emu_renderer, 30, 30, 30, 255); // Darker border around screen
    SDL_Rect borderRect = { 
        EMU_SCREEN_X - EMU_SCREEN_BORDER, 
        EMU_SCREEN_Y - EMU_SCREEN_BORDER, 
        EMU_SCREEN_W + 2 * EMU_SCREEN_BORDER, 
        EMU_SCREEN_H + 2 * EMU_SCREEN_BORDER 
    };
    SDL_RenderFillRect(emu_renderer, &borderRect);

    // 4. Draw e-ink screen content
    SDL_Rect destRect = { EMU_SCREEN_X, EMU_SCREEN_Y, EMU_SCREEN_W, EMU_SCREEN_H };
    SDL_RenderCopy(emu_renderer, emu_screen_texture, NULL, &destRect);

    // 5. Draw buttons
    for (int i = 0; i < BTN_COUNT; i++) {
        const emu_ui_button_t *btn = &emu_ui_buttons[i];
        bool pressed = hal_gpio_is_pressed(btn->btn_index);

        if (pressed) {
            SDL_SetRenderDrawColor(emu_renderer, 255, 50, 50, 255); // Red-ish when pressed
        } else {
            SDL_SetRenderDrawColor(emu_renderer, 80, 80, 80, 255); // Darker gray when idle
        }

        SDL_Rect btnRect = { btn->x, btn->y, btn->w, btn->h };
        SDL_RenderFillRect(emu_renderer, &btnRect);
        
        // Draw button border
        SDL_SetRenderDrawColor(emu_renderer, 150, 150, 150, 255);
        SDL_RenderDrawRect(emu_renderer, &btnRect);
    }

    SDL_RenderPresent(emu_renderer);
}

void emu_ui_quit(void) {
    if (emu_logo_texture) {
        SDL_DestroyTexture(emu_logo_texture);
        emu_logo_texture = NULL;
    }
}
