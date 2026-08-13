#pragma once

#include <cstdint>

#include "defines.h"
#include "angel_types.h"

namespace angel {
// @temp ANGEL_API
class Input {
   public:
#include "input_defines.h"
    void init_input();
    void deinit_input();
    void input_update();

    void process_key(Keycode key, bool pressed);
    void process_mouse_button(MouseButton button, bool pressed);
    void process_mouse_position(int x, int y);
    void process_mouse_wheel(int delta);

    // @todo: for debugging purposes only. remove.
    void print_pressed_keys();
};
}  // namespace angel
