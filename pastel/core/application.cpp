#include "application.h"
#include "core/logging/logger.h"
#include "core/memory/memory.h"
#include "core/platform/platform.h"

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
    WindowConfig config = {
        .x                = 0,
        .y                = 0,
        .width            = 1920,
        .height           = 1080,
        .application_name = "PASTEL Engine",
    };

    WindowState window_state = WindowState();
    Logger::logger_init(&window_state);
    if (!window_state.open_window(config)) {
        CORE_LOG_FATAL("Failed to open window")
        return false;
    }

    // @temp:
    CORE_LOG_FATAL("FATAL MESSAGE TEST")
    CORE_LOG_ERROR("ERROR MESSAGE TEST")
    CORE_LOG_WARN("WARN MESSAGE TEST")
    CORE_LOG_INFO("INFO MESSAGE TEST")
    CORE_LOG_DEBUG("DEBUG MESSAGE TEST")
    CORE_LOG_TRACE("TRACE MESSAGE TEST")

    window_state.clear_terminal_colour();

    int i = 0;
    (void)i;
    while (window_state.running) {
        i++;
        window_state.update();
        if (!window_state.pump_window()) {
            window_state.running = false;
        }

        // CORE_LOG_INFO("index: %d, delta time: %f, fps: %f\n",
        //               i,
        //               window_state.get_delta_time(),
        //               1.0 / window_state.get_delta_time())

        // @temp
        // window_state.input.print_pressed_keys();
    }
    return true;
}

}  // namespace Pastel
