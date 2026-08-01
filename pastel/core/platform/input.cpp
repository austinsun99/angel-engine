#include "input.h"

#include <cstring>
#include <iostream>

struct InputState {
    bool keys[Keycode::MAX_KEYS];
    bool mouse_buttons[MouseButton::MAX_BUTTONS];
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
    current_state.mouse_buttons[button] = pressed;
}

// @todo: for debugging purposes only. remove.
void print_pressed_keys() {
    for (int i = 0; i < Keycode::MAX_KEYS; i++) {
        if (current_state.keys[i]) {
            std::cout << "keycode pressed: " << i << std::endl;
        }
    }

    for (int i = 0; i < MouseButton::MAX_BUTTONS; i++) {
        if (current_state.mouse_buttons[i]) {
            std::cout << "button pressed: " << i << std::endl;
        }
    }
}
