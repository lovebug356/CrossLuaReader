#pragma once

#include <SDL2/SDL.h>
#include "hal_gpio.h"

// Display dimensions (Portrait)
#define EMU_DISP_WIDTH  480
#define EMU_DISP_HEIGHT 800

// Generic Button size
#define EMU_BTN_W 100
#define EMU_BTN_H 25
#define EMU_SCREEN_BORDER 5

// UI Layout constants
#define EMU_MARGIN_TOP    100
#define EMU_MARGIN_LEFT   10
#define EMU_MARGIN_BOTTOM (EMU_MARGIN_LEFT + EMU_BTN_H + EMU_SCREEN_BORDER)
#define EMU_MARGIN_RIGHT  (EMU_MARGIN_LEFT + EMU_BTN_H + EMU_SCREEN_BORDER)

// Screen position and border
#define EMU_SCREEN_X      EMU_MARGIN_LEFT
#define EMU_SCREEN_Y      EMU_MARGIN_TOP
#define EMU_SCREEN_W      EMU_DISP_WIDTH
#define EMU_SCREEN_H      EMU_DISP_HEIGHT

// Window dimensions derived from display and margins
#define EMU_WINDOW_WIDTH  (EMU_MARGIN_LEFT + EMU_SCREEN_W + EMU_MARGIN_RIGHT)
#define EMU_WINDOW_HEIGHT (EMU_MARGIN_TOP + EMU_SCREEN_H + EMU_MARGIN_BOTTOM)

// Button dimensions and spacing
#define EMU_BTN_W_BOTTOM          EMU_BTN_W
#define EMU_BTN_H_BOTTOM          EMU_BTN_H
#define EMU_BTN_GAP_BOTTOM        10
#define EMU_BTN_GAP_SCREEN_BOTTOM 10
#define EMU_BTN_Y_BOTTOM          (EMU_SCREEN_Y + EMU_SCREEN_H + EMU_BTN_GAP_SCREEN_BOTTOM)

#define EMU_BTN_W_SIDE            EMU_BTN_H
#define EMU_BTN_H_SIDE            EMU_BTN_W
#define EMU_BTN_H_POWER           40
#define EMU_BTN_GAP_SCREEN_SIDE   10
#define EMU_BTN_GAP_SIDE_BTNS     15
#define EMU_BTN_X_SIDE            (EMU_SCREEN_X + EMU_SCREEN_W + EMU_BTN_GAP_SCREEN_SIDE)

// Define button positions matching Xteink-X4 layout
// Centering 4 buttons below screen
#define EMU_BTN_TOTAL_WIDTH_BOTTOM (4 * EMU_BTN_W_BOTTOM + 3 * EMU_BTN_GAP_BOTTOM)
#define EMU_BTN_BOTTOM_START_X     (EMU_SCREEN_X + (EMU_SCREEN_W - EMU_BTN_TOTAL_WIDTH_BOTTOM)/2)

// Bottom button X positions
#define EMU_BTN_BACK_X    (EMU_BTN_BOTTOM_START_X)
#define EMU_BTN_CONFIRM_X (EMU_BTN_BOTTOM_START_X + 1 * (EMU_BTN_W_BOTTOM + EMU_BTN_GAP_BOTTOM))
#define EMU_BTN_LEFT_X    (EMU_BTN_BOTTOM_START_X + 2 * (EMU_BTN_W_BOTTOM + EMU_BTN_GAP_BOTTOM))
#define EMU_BTN_RIGHT_X   (EMU_BTN_BOTTOM_START_X + 3 * (EMU_BTN_W_BOTTOM + EMU_BTN_GAP_BOTTOM))

// Side button Y positions (centered vertically)
#define EMU_BTN_UP_Y      (EMU_SCREEN_Y + (EMU_SCREEN_H / 2) - EMU_BTN_H_SIDE - (EMU_BTN_GAP_SIDE_BTNS / 2))
#define EMU_BTN_DOWN_Y    (EMU_SCREEN_Y + (EMU_SCREEN_H / 2) + (EMU_BTN_GAP_SIDE_BTNS / 2))

typedef struct {
    int x, y, w, h;
    int btn_index;
    const char* label;
} emu_ui_button_t;

static const emu_ui_button_t emu_ui_buttons[BTN_COUNT] = {
    // 4 below the screen: BACK, CONFIRM, LEFT, RIGHT
    {EMU_BTN_BACK_X,    EMU_BTN_Y_BOTTOM, EMU_BTN_W_BOTTOM, EMU_BTN_H_BOTTOM, BTN_BACK,    "BACK"},
    {EMU_BTN_CONFIRM_X, EMU_BTN_Y_BOTTOM, EMU_BTN_W_BOTTOM, EMU_BTN_H_BOTTOM, BTN_CONFIRM, "CONFIRM"},
    {EMU_BTN_LEFT_X,    EMU_BTN_Y_BOTTOM, EMU_BTN_W_BOTTOM, EMU_BTN_H_BOTTOM, BTN_LEFT,    "LEFT"},
    {EMU_BTN_RIGHT_X,   EMU_BTN_Y_BOTTOM, EMU_BTN_W_BOTTOM, EMU_BTN_H_BOTTOM, BTN_RIGHT,   "RIGHT"},
    
    // Side buttons
    {EMU_BTN_X_SIDE, EMU_BTN_UP_Y,   EMU_BTN_W_SIDE, EMU_BTN_H_SIDE,  BTN_UP,    "UP"},
    {EMU_BTN_X_SIDE, EMU_BTN_DOWN_Y, EMU_BTN_W_SIDE, EMU_BTN_H_SIDE,  BTN_DOWN,  "DOWN"},
    
    // Top right
    {EMU_BTN_X_SIDE, EMU_SCREEN_Y,   EMU_BTN_W_SIDE, EMU_BTN_H_POWER, BTN_POWER, "POWER"},
};

/** Initialize the emulator UI components. */
void emu_ui_init(SDL_Renderer *renderer, SDL_Texture *texture);

/** Update the emulator UI (bezel, logo, and buttons). */
void emu_ui_update(void);

/** Cleanup UI resources. */
void emu_ui_quit(void);
