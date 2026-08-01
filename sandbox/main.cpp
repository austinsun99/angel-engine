#include <core/platform/platform.h>

int main() {
    WindowConfig config = {.width = 1920, .height = 1080, .application_name = "PASTEL Engine"};
    WindowState window_state = init_window(config);
    open_window(&window_state);

    while (pump_window(&window_state)) {
    }
    deinit_window(&window_state);
}
