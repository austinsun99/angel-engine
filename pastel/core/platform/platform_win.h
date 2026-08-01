#pragma once

#include "platform.h"

WindowState init_window_win();
bool open_window_win(const WindowConfig config, WindowState *const state);
bool pump_window_win(WindowState *const state);
void deinit_window_win(WindowState *const state);
