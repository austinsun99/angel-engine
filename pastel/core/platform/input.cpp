#include "input.h"

#include <cstdint>
#include <cstring>
#include <iostream>

using namespace Pastel;

struct InputState {
    bool keys[Input::Keycode::MAX_KEYS];
    bool mouse_button[Input::MouseButton::MAX_BUTTONS];

    // An absolute value of 120 indicates "one line".
    // A positive value indicates the mouse wheel scrolling away from the user.
    // A negative value indicates the mouse wheel scrolling towards the user.
    int mouse_wheel_delta;
    uint16_t mouse_x;
    uint16_t mouse_y;
};

static InputState previous_state;
static InputState current_state;

void Input::init_input() {
    previous_state = {};
    current_state  = {};

    std::memset(&previous_state, 0, sizeof(InputState));
    std::memset(&current_state, 0, sizeof(InputState));
}

void Input::deinit_input() {
}

void Input::input_update() {
    std::memcpy(&previous_state, &current_state, sizeof(InputState));
    current_state.mouse_wheel_delta = 0;
}

void Input::process_key(Keycode key, bool pressed) {
    current_state.keys[key] = pressed;
}

void Input::process_mouse_button(MouseButton button, bool pressed) {
    current_state.mouse_button[button] = pressed;
}

void Input::process_mouse_position(uint16_t x, uint16_t y) {
    current_state.mouse_x = x;
    current_state.mouse_y = y;
}

void Input::process_mouse_wheel(int delta) {
    current_state.mouse_wheel_delta = delta;
}

// @todo: for debugging purposes only. remove.
void Input::print_pressed_keys() {
    for (int i = 0; i < Keycode::MAX_KEYS; i++) {
        if (current_state.keys[i] != previous_state.keys[i]) {
            std::cout << "keycode pressed: " << i << std::endl;
        }
    }

    for (int i = 0; i < MouseButton::MAX_BUTTONS; i++) {
        if (current_state.mouse_button[i] != previous_state.keys[i]) {
            std::cout << "button pressed: " << i << std::endl;
        }
    }

    if (current_state.mouse_x != previous_state.mouse_x) std::cout << "mouse x: " << current_state.mouse_x << std::endl;
    if (current_state.mouse_y != previous_state.mouse_y) std::cout << "mouse y: " << current_state.mouse_y << std::endl;
    if (current_state.mouse_wheel_delta != 0)
        std::cout << "mouse wheel delta: " << current_state.mouse_wheel_delta << std::endl;
}
