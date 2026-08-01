#include "input.h"

#include <cstdint>
#include <cstring>
#include <iostream>

struct InputState {
    bool keys[Keycode::MAX_KEYS];
    bool mouse_button[MouseButton::MAX_BUTTONS];
    uint16_t mouse_x;
    uint16_t mouse_y;
};

static InputState previous_state;
static InputState current_state;

void init_input() {
    previous_state = {};
    current_state = {};

    std::memset(&previous_state, 0, sizeof(InputState));
    std::memset(&current_state, 0, sizeof(InputState));
}

void deinit_input() {
}

void input_update() {
    std::memcpy(&previous_state, &current_state, sizeof(InputState));
}

void process_key(Keycode key, bool pressed) {
    current_state.keys[key] = pressed;
}

void process_mouse_button(MouseButton button, bool pressed) {
    current_state.mouse_button[button] = pressed;
}

void process_mouse_position(uint16_t x, uint16_t y) {
    current_state.mouse_x = x;
    current_state.mouse_y = y;
}

// @todo: for debugging purposes only. remove.
void print_pressed_keys() {
    for (int i = 0; i < Keycode::MAX_KEYS; i++) {
        if (current_state.keys[i]) {
            std::cout << "keycode pressed: " << i << std::endl;
        }
    }

    for (int i = 0; i < MouseButton::MAX_BUTTONS; i++) {
        if (current_state.mouse_button[i]) {
            std::cout << "button pressed: " << i << std::endl;
        }
    }

    std::cout << "mouse x: " << current_state.mouse_x << std::endl;
    std::cout << "mouse y: " << current_state.mouse_y << std::endl;
}
