#pragma once

// @todo: place into namespace

#include <cstdint>

#include "defines.h"

namespace Pastel {
// @temp PASTEL_API
class Input {
   public:
#include "input_defines.h"
    PASTEL_API void init_input();
    PASTEL_API void deinit_input();
    PASTEL_API void input_update();

    void process_key(Keycode key, bool pressed);
    void process_mouse_button(MouseButton button, bool pressed);
    void process_mouse_position(uint16_t x, uint16_t y);
    void process_mouse_wheel(int delta);

    // @todo: for debugging purposes only. remove.
    PASTEL_API void print_pressed_keys();
};
}  // namespace Pastel
