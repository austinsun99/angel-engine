#pragma once

#include "defines.h"

// @todo: place into namespace

struct WindowConfig {
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
PASTEL_API WindowState init_window(WindowConfig config);
PASTEL_API bool open_window(WindowState *const state);
PASTEL_API bool pump_window(WindowState *const state);
PASTEL_API void deinit_window(WindowState *const state);
