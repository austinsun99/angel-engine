#include "application.h"
#include "containers/vector.hpp"
#include "core/logging/logger.h"
#include "core/memory/memory.h"
#include "core/platform/input.h"
#include "core/platform/platform.h"
#include "pastel_types.h"

namespace Pastel {
bool application_lifecycle();

bool start_application() {
    Memory::init_memory();
    if (!application_lifecycle()) {
        CORE_LOG_FATAL("Could not start application")
    }
    Memory::deinit_memory();
    return true;
}

bool application_lifecycle() {
    Platform::WindowConfig config = {
        .x                = 0,
        .y                = 0,
        .width            = 1920,
        .height           = 1080,
        .application_name = "PASTEL Engine",
    };

    Platform::WindowState window_state = Platform::WindowState();
    Logger::logger_init();
    if (!window_state.open_window(config)) {
        CORE_LOG_FATAL("Failed to open window")
        return false;
    }

    Platform::clear_terminal_colour();

    while (window_state.running) {
        window_state.update();
        if (!window_state.pump_window()) {
            window_state.running = false;
        }

        window_state.input.print_pressed_keys();
    }
    return true;
}

}  // namespace Pastel
