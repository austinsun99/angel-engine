#include <core/platform/input.h>
#include <core/platform/platform.h>

int main() {
    WindowConfig config = {
        .x                = 0,
        .y                = 0,
        .width            = 1920,
        .height           = 1080,
        .application_name = "PASTEL Engine",
    };
    WindowState window_state = init_window();
    init_input();
    open_window(config, &window_state);

    while (window_state.running) {
        if (!pump_window(&window_state)) {
            window_state.running = false;
        }
        input_update();

        // @temp
        print_pressed_keys();
    }
    deinit_input();
    deinit_window(&window_state);
}
