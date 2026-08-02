#pragma once

#include "platform.h"

WindowState init_window_wayland();
bool open_window_wayland(const WindowConfig config, WindowState *const state);
bool pump_window_wayland(WindowState *const state);
void deinit_window_wayland(WindowState *const state);
