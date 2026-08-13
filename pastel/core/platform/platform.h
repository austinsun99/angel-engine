#pragma once

#include <vulkan/vulkan_core.h>
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

class Window {
   public:
    bool running;
    u32 width;
    u32 height;

    Input input;

    Window();
    ~Window() = default;
    void deinit();
    void update();

    bool open_window(const WindowConfig config);
    bool pump_window();

    void *get_internal_state();
    void on_window_close();

    bool create_vulkan_surface(VkInstance const &vulkan_instance,
                               VkAllocationCallbacks *const &custom_allocator,
                               VkSurfaceKHR *out_vulkan_surface) const;

    u32 framebuffer_width() const {
        return width;
    }

    u32 framebuffer_height() const {
        return height;
    }

   private:
    bool console_initialized;
    void *internal_state;
};

double get_time();

// @todo: use templates for compile time colour?
// @todo: allow specifying stdout vs stderr
void print_terminal(const char *msg,
                    Io::TerminalColour fg = Io::TERMINAL_COLOUR_WHITE,
                    Io::TerminalColour bg = Io::TERMINAL_COLOUR_NONE);
void console_write(const char *msg);
void clear_terminal_colour();

}  // namespace Pastel::Platform
