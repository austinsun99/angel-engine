#include <core/platform/input.h>
#include <core/platform/platform.h>

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

    while (window_state.running) {
        window_state.input.input_update();
        if (!window_state.pump_window()) {
            window_state.running = false;
        }

        // @temp
        window_state.input.print_pressed_keys();
    }
}
