#include "platform.h"

#include "defines.h"
#include "platform_win.h"

WindowState init_window() {
#if defined(PLATFORM_WINDOWS)
    return init_window_win();
#elif defined(PLATFORM_LINUX)
    // @todo:
#    error "Not implemented yet"
#endif
}

bool open_window(const WindowConfig config, WindowState *const state) {
#if defined(PLATFORM_WINDOWS)
    return open_window_win(config, state);
#elif defined(PLATFORM_LINUX)
    // @todo:
#    error "Not implemented yet"
#endif
}

bool pump_window(WindowState *const state) {
#if defined(PLATFORM_WINDOWS)
    return pump_window_win(state);
#elif defined(PLATFORM_LINUX)
    // @todo:
#    error "Not implemented yet"
#endif
}

void deinit_window(WindowState *const state) {
#if defined(PLATFORM_WINDOWS)
    deinit_window_win(state);
#elif defined(PLATFORM_LINUX)
    // @todo:
#    error "Not implemented yet"
#endif
}
