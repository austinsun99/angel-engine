#pragma once

#include "platform.h"

WindowState init_window_win(WindowConfig config);
bool open_window_win(WindowState *const state);
bool pump_window_win(WindowState *const state);
void deinit_window_win(WindowState *const state);
