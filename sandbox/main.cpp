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
    open_window(config, &window_state);
    while (pump_window(&window_state));
    deinit_window(&window_state);
}
