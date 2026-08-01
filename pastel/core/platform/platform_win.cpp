#include "platform_win.h"
#include <iostream>

struct InternalState {
    int x;
};

WindowState init_window_win(WindowConfig config) {
    InternalState *internal = new InternalState();
    internal->x = 5;

    WindowState state = {.width = config.width, .height = config.height, .internal_state = internal};
    return state;
}

bool open_window_win(WindowState *const state) {
    InternalState *internal = static_cast<InternalState *>(state->internal_state);
    std::cout << internal->x << std::endl;
    return true;
}

bool pump_window_win(WindowState *const state) {
    return true;
}

void deinit_window_win(WindowState *const state) {
    delete static_cast<InternalState *>(state->internal_state);
}
