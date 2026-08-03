#include <core/platform/input.h>
#include <core/platform/platform.h>
#include <core/logging/logger.h>

using namespace Pastel;

int main() {
    WindowConfig config = {
        .x                = 0,
        .y                = 0,
        .width            = 1920,
        .height           = 1080,
        .application_name = "PASTEL Engine",
    };

    WindowState window_state = WindowState();
    window_state.open_window(config);
    Logger::logger_init(&window_state);

    CORE_LOG_FATAL("FATAL MESSAGE TEST")
    CORE_LOG_ERROR("ERROR MESSAGE TEST")
    CORE_LOG_WARN("WARN MESSAGE TEST")
    CORE_LOG_INFO("INFO MESSAGE TEST")
    CORE_LOG_DEBUG("DEBUG MESSAGE TEST")
    CORE_LOG_TRACE("TRACE MESSAGE TEST")

    int i = 0;
    (void)i;
    while (window_state.running) {
        i++;
        window_state.update();
        if (!window_state.pump_window()) {
            window_state.running = false;
        }

        // CORE_LOG_TRACE("index: %d, delta time: %f, fps: %f\n",
        //          i,
        //          window_state.get_delta_time(),
        //          1.0 / window_state.get_delta_time())
        // window_state.get_delta_time());

        // @temp
        window_state.input.print_pressed_keys();
    }
}
