#pragma once

#include "core/platform/input.h"
#include "core/io/terminal_colours.h"

namespace Pastel::Platform {
struct WindowConfig {
    int x;
    int y;
    int width;
    int height;
    const char *application_name;
};

// @temp: remove PASTEL_API (here for testing purposes)
// @todo: change name since this does not only handle window state but all platform related items
class WindowState {
   public:
    bool running;
    int width;
    int height;

    Input input;

    WindowState();
    ~WindowState();
    void update();

    bool open_window(const WindowConfig config);
    bool pump_window();

    // Obtains the current time. The starting point of the time is unspecified.
    // This method is used for getting differences in time.
    double get_time();
    double get_delta_time();

    void *get_internal_state();
    void on_window_close();

    bool console_is_initialized();

   private:
    bool console_initialized;
    double current_time;
    double prev_time;
    void *internal_state;
};

// @todo: use templates for compile time colour?
// @todo: allow specifying stdout vs stderr
void print_terminal(const char *msg,
                    Io::TerminalColour fg = Io::TERMINAL_COLOUR_WHITE,
                    Io::TerminalColour bg = Io::TERMINAL_COLOUR_NONE);
void console_write(const char *msg);
void clear_terminal_colour();

}  // namespace Pastel::Platform
