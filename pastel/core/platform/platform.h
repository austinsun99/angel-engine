#pragma once

#include "core/platform/input.h"
#include "core/io/terminal_colours.h"
#include "defines.h"

namespace Pastel {
struct PASTEL_API WindowConfig {
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

    // @todo: use templates for compile time colour?
    void print_terminal(const char *msg,
                        Io::TerminalColour fg = Io::TERMINAL_COLOUR_WHITE,
                        Io::TerminalColour bg = Io::TERMINAL_COLOUR_NONE);

   private:
    void *internal_state;
};
}  // namespace Pastel
