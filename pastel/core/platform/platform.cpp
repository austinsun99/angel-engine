#include "platform.h"

#include "defines.h"

#ifdef PLATFORM_LINUX
#    include "platform_wayland.h"
#endif

#ifdef PLATFORM_WINDOWS
#    include "platform_win.h"
#endif

// @todo: x11, macos
WindowState init_window() {
#if defined(PLATFORM_WINDOWS)
    return init_window_win();
#elif defined(PLATFORM_LINUX)
    return init_window_wayland();
#endif
}

bool open_window(const WindowConfig config, WindowState *const state) {
#if defined(PLATFORM_WINDOWS)
    return open_window_win(config, state);
#elif defined(PLATFORM_LINUX)
    return open_window_wayland(config, state);
#endif
}

bool pump_window(WindowState *const state) {
#if defined(PLATFORM_WINDOWS)
    return pump_window_win(state);
#elif defined(PLATFORM_LINUX)
    return pump_window_wayland(state);
#endif
}

void deinit_window(WindowState *const state) {
#if defined(PLATFORM_WINDOWS)
    deinit_window_win(state);
#elif defined(PLATFORM_LINUX)
    deinit_window_wayland(state);
#endif
}
