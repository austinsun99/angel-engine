#pragma once

#include "core/platform/input.h"
#include "defines.h"

namespace Pastel {
struct WindowConfig {
    int x;
    int y;
    int width;
    int height;
    const char *application_name;
};

// @temp: remove PASTEL_API (here for testing purposes)
class PASTEL_API WindowState {
   public:
    bool running;
    int width;
    int height;

    Input input;

    WindowState();
    ~WindowState();
    bool open_window(const WindowConfig config);
    bool pump_window();

    // @todo: turn colour into an enum
    void print_terminal(const char *msg, int colour_fg, int colour_bg);

   private:
    void *internal_state;
};
}  // namespace Pastel
