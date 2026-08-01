#pragma once

// @todo: place into namespace

#include "defines.h"
enum MouseButton {
    LEFT,
    RIGHT,
    MIDDLE,
    MAX_BUTTONS,
};

// The keycodes listed here match the virtual key codes defined by Winuser.h
enum Keycode {
    BACK = 0x08,
    TAB = 0x09,

    CLEAR = 0x0C,
    RETURN = 0x0D,

    SHIFT = 0x10,
    CONTROL,
    ALT,
    PAUSE,
    CAPSLOCK,

    ESCAPE = 0x1B,
    SPACE = 0x20,
    PAGEUP,
    PAGEDOWN,
    END,
    HOME,
    LEFTARROW,
    UPARROW,
    RIGHTARROW,
    DOWNARROW,
    SELECT,
    PRINT,
    EXECUTE,
    PRINTSCRN,
    INSERT,
    DELETE,
    HELP,

    // The keys 0-9 and A-Z match the ASCII equivalents
    KEY_0 = 0x30,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    A = 0x41,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    LSUP = 0x5B,  // The left windows logo key
                  // @todo check equivalent for wayland
    RSUP,         // @todo (same as above)

    NUMPAD0 = 0x60,
    NUMPAD1,
    NUMPAD2,
    NUMPAD3,
    NUMPAD4,
    NUMPAD5,
    NUMPAD6,
    NUMPAD7,
    NUMPAD8,
    NUMPAD9,

    MULTIPLY = 0x6A,
    ADD,
    SEPARATOR,
    SUBTRACT,
    DECIMAL,
    DIVIDE,

    F1 = 0x70,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,

    NUMLOCK = 0x90,
    SCROLL,

    LSHIFT = 0xA0,
    RSHIFT,
    LCTRL,
    RCTRL,  // for the canadian CSA keyboard, the right ctrl key is also 0xDF,
    LALT,
    RALT,

    VOLUME_MUTE = 0xAD,
    VOLUME_DOWN,
    VOLUME_UP,

    // For windows: the keys marked with !US is only accurate for the US ANSI keyboard
    SEMICOLON = 0xBA,  // !US
    PLUS,
    COMMA,
    MINUS,
    PERIOD,
    FSLASH,         // !US
    GRAVE,          // !US
    LBRACE = 0xDB,  // !US
    BACKSLASH,      // !US
    RBRACE,
    APOSTROPHE,

    MAX_KEYS,
};

// @temp PASTEL_API
PASTEL_API void init_input();
PASTEL_API void deinit_input();
PASTEL_API void input_update();

void process_key(Keycode key, bool pressed);
void process_mouse_button(MouseButton button, bool pressed);

// @todo: for debugging purposes only. remove.
PASTEL_API void print_pressed_keys();
