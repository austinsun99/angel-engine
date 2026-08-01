#pragma once

#include "defines.h"

// @todo: place into namespace

struct WindowConfig {
    int x;
    int y;
    int width;
    int height;
    const char* application_name;
};

struct WindowState {
    int width;
    int height;
    void *internal_state;
};

// @temp: remove PASTEL_API (here for testing purposes)
PASTEL_API WindowState init_window();
PASTEL_API bool open_window(const WindowConfig config, WindowState *const state);
PASTEL_API bool pump_window(WindowState *const state);
PASTEL_API void deinit_window(WindowState *const state);

// @todo: turn colour into an enum
void print_terminal(const char* msg, int colour_fg, int colour_bg);
