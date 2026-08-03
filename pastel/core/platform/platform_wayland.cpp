#include "defines.h"
#ifdef PLATFORM_LINUX

#    include "xdg-shell-client-protocol.h"
#    include "core/io/terminal_colours.h"
#    include "core/platform/platform.h"
#    include "platform.h"
#    include "input.h"

#    include <wayland-util.h>

#    include <sys/syscall.h>
#    include <sys/mman.h>
#    include <cstdint>
#    include <cstdio>
#    include <poll.h>
#    include <cerrno>
#    include <ctime>
#    include <unistd.h>

#    include <linux/input-event-codes.h>
#    include <xkbcommon/xkbcommon.h>
#    include <xkbcommon/xkbcommon-keysyms.h>
#    include <wayland-client-core.h>
#    include <wayland-client-protocol.h>

#    include <algorithm>
#    include <cstring>

using namespace Pastel;

struct InternalState {
    struct wl_display *wl_display;
    struct wl_registry *wl_registry;
    struct wl_compositor *wl_compositor;
    struct wl_shm *wl_shm;  // @temp

    struct wl_seat *wl_seat;
    struct wl_pointer *wl_pointer;
    struct wl_keyboard *wl_keyboard;

    struct xdg_wm_base *xdg_wm_base;

    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;

    struct xkb_state *xkb_state;
    struct xkb_context *xkb_context;
    struct xkb_keymap *xkb_keymap;
};

const static struct {
    static void ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial) {
        (void)data;
        xdg_wm_base_pong(xdg_wm_base, serial);
    }
    const xdg_wm_base_listener listener = xdg_wm_base_listener{
        .ping = ping,
    };
} xdg_wm_base_listener;

const static struct {
    static void configure(void *data,
                          struct xdg_toplevel *xdg_toplevel,
                          int32_t width,
                          int32_t height,
                          struct wl_array *states) {
        (void)xdg_toplevel;
        (void)states;
        if (width == 0 || height == 0) return;

        WindowState *state = static_cast<WindowState *>(data);
        state->width       = width;
        state->height      = height;
    }
    static void close(void *data, struct xdg_toplevel *xdg_toplevel) {
        (void)xdg_toplevel;
        WindowState *state = static_cast<WindowState *>(data);

        state->running = false;
    }
    static void configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height) {
        (void)data;
        (void)xdg_toplevel;
        (void)width;
        (void)height;
    }
    static void wm_capabilities(void *data, struct xdg_toplevel *xdg_toplevel, struct wl_array *capabilities) {
        (void)data;
        (void)xdg_toplevel;
        (void)capabilities;
    }
    const xdg_toplevel_listener listener = xdg_toplevel_listener{.configure        = configure,
                                                                 .close            = close,
                                                                 .configure_bounds = configure_bounds,
                                                                 .wm_capabilities  = wm_capabilities};
} xdg_toplevel_listener;

const static struct {
    static void enter(void *data,
                      struct wl_pointer *wl_pointer,
                      uint32_t serial,
                      struct wl_surface *surface,
                      wl_fixed_t surface_x,
                      wl_fixed_t surface_y) {
        (void)wl_pointer;
        (void)serial;
        (void)surface;

        WindowState *window_state = static_cast<WindowState *>(data);
        const uint16_t mouse_x    = wl_fixed_to_int(surface_x);
        const uint16_t mouse_y    = wl_fixed_to_int(surface_y);
        window_state->input.process_mouse_position(mouse_x, mouse_y);
    }

    static void leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *surface) {
        (void)data;
        (void)wl_pointer;
        (void)serial;
        (void)surface;
    }
    static void motion(void *data,
                       struct wl_pointer *wl_pointer,
                       uint32_t time,
                       wl_fixed_t surface_x,
                       wl_fixed_t surface_y) {
        (void)wl_pointer;
        (void)time;
        WindowState *window_state = static_cast<WindowState *>(data);
        const uint16_t mouse_x    = wl_fixed_to_int(surface_x);
        const uint16_t mouse_y    = wl_fixed_to_int(surface_y);

        // @fix: mouse position goes to ~65565 when nearing edge of window,
        // supposedly because of wraparound.
        window_state->input.process_mouse_position(mouse_x, mouse_y);
    }

    static void button(void *data,
                       struct wl_pointer *wl_pointer,
                       uint32_t serial,
                       uint32_t time,
                       uint32_t button,
                       uint32_t state) {
        (void)wl_pointer;
        (void)serial;
        (void)time;
        WindowState *window_state       = static_cast<WindowState *>(data);
        bool pressed                    = state == WL_POINTER_BUTTON_STATE_PRESSED;
        Input::MouseButton mouse_button = Input::MAX_BUTTONS;
        switch (button) {
            case BTN_LEFT:
                mouse_button = Input::MOUSE_LEFT;
                break;
            case BTN_RIGHT:
                mouse_button = Input::MOUSE_RIGHT;
                break;
            case BTN_MIDDLE:
                mouse_button = Input::MOUSE_MIDDLE;
                break;
        }

        window_state->input.process_mouse_button(mouse_button, pressed);
    }

    static void axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
        (void)data;
        (void)wl_pointer;
        (void)time;
        (void)axis;
        (void)value;

        // @todo
    }

    static void frame(void *data, struct wl_pointer *wl_pointer) {
        (void)data;
        (void)wl_pointer;
    }

    static void axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source) {
        (void)data;
        (void)wl_pointer;
        (void)axis_source;
    }

    static void axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis) {
        (void)data;
        (void)wl_pointer;
        (void)time;
        (void)axis;
    }

    static void axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete) {
        (void)data;
        (void)wl_pointer;
        (void)axis;
        (void)discrete;
    }

    static void axis_value120(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t value120) {
        (void)data;
        (void)wl_pointer;
        (void)axis;
        (void)value120;
    }

    static void axis_relative_direction(void *data, struct wl_pointer *wl_pointer, uint32_t axis, uint32_t direction) {
        (void)data;
        (void)wl_pointer;
        (void)axis;
        (void)direction;
    }

    const wl_pointer_listener listener = wl_pointer_listener{
        .enter                   = enter,
        .leave                   = leave,
        .motion                  = motion,
        .button                  = button,
        .axis                    = axis,
        .frame                   = frame,
        .axis_source             = axis_source,
        .axis_stop               = axis_stop,
        .axis_discrete           = axis_discrete,
        .axis_value120           = axis_value120,
        .axis_relative_direction = axis_relative_direction,
    };
} wl_pointer_listener;

const static struct {
    static void keymap(void *data, struct wl_keyboard *wl_keyboard, uint32_t format, int32_t fd, uint32_t size) {
        (void)wl_keyboard;
        (void)format;

        WindowState *state      = static_cast<WindowState *>(data);
        InternalState *internal = static_cast<InternalState *>(state->get_internal_state());
        // @todo: assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1)

        char *map_shm = static_cast<char *>(mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0));

        // @todo: assert(map_shm != MAP_FAILED)
        // @todo: assert(internal->xkb_context)
        xkb_keymap *keymap = xkb_keymap_new_from_string(internal->xkb_context,
                                                        map_shm,
                                                        XKB_KEYMAP_FORMAT_TEXT_V1,
                                                        XKB_KEYMAP_COMPILE_NO_FLAGS);
        munmap(map_shm, size);
        close(fd);

        xkb_state *xkb_state = xkb_state_new(keymap);
        xkb_keymap_unref(internal->xkb_keymap);
        xkb_state_unref(internal->xkb_state);
        internal->xkb_keymap = keymap;
        internal->xkb_state  = xkb_state;
    }

    static void enter(void *data,
                      struct wl_keyboard *wl_keyboard,
                      uint32_t serial,
                      struct wl_surface *surface,
                      struct wl_array *keys) {
        (void)data;
        (void)wl_keyboard;
        (void)serial;
        (void)surface;
        (void)keys;
    }

    static void leave(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, struct wl_surface *surface) {
        (void)data;
        (void)wl_keyboard;
        (void)serial;
        (void)surface;
    }
    static Input::Keycode xkb_to_input(xkb_keysym_t xkb_sym) {
        switch (xkb_sym) {
            case XKB_KEY_BackSpace:
                return Input::BACK;
            case XKB_KEY_Tab:
                return Input::TAB;
            case XKB_KEY_Clear:
                return Input::CLEAR;
            case XKB_KEY_Return:
                return Input::RETURN;

            case XKB_KEY_Control_L:
                return Input::LCTRL;
            case XKB_KEY_Control_R:
                return Input::RCTRL;

            case XKB_KEY_Alt_L:
                return Input::LALT;
            case XKB_KEY_Alt_R:
                return Input::RALT;

            case XKB_KEY_Shift_L:
                return Input::LSHIFT;
            case XKB_KEY_Shift_R:
                return Input::RSHIFT;

            case XKB_KEY_Pause:
                return Input::PAUSE;
            case XKB_KEY_Caps_Lock:
                return Input::CAPSLOCK;

            case XKB_KEY_Escape:
                return Input::ESCAPE;
            case XKB_KEY_space:
                return Input::SPACE;
            case XKB_KEY_Page_Up:
                return Input::PAGEUP;
            case XKB_KEY_Page_Down:
                return Input::PAGEDOWN;
            case XKB_KEY_End:
                return Input::END;
            case XKB_KEY_Home:
                return Input::HOME;
            case XKB_KEY_leftarrow:
                return Input::LEFTARROW;
            case XKB_KEY_uparrow:
                return Input::UPARROW;
            case XKB_KEY_rightarrow:
                return Input::RIGHTARROW;
            case XKB_KEY_downarrow:
                return Input::DOWNARROW;
            case XKB_KEY_Select:
                return Input::SELECT;
            case XKB_KEY_Print:
                return Input::PRINT;
            case XKB_KEY_Execute:
                return Input::EXECUTE;
            case XKB_KEY_Insert:
                return Input::INSERT;
            case XKB_KEY_Delete:
                return Input::DEL;
            case XKB_KEY_Help:
                return Input::HELP;
            case XKB_KEY_0:
            case XKB_KEY_1:
            case XKB_KEY_2:
            case XKB_KEY_3:
            case XKB_KEY_4:
            case XKB_KEY_5:
            case XKB_KEY_6:
            case XKB_KEY_7:
            case XKB_KEY_8:
            case XKB_KEY_9:
            case XKB_KEY_a:
            case XKB_KEY_b:
            case XKB_KEY_c:
            case XKB_KEY_d:
            case XKB_KEY_e:
            case XKB_KEY_f:
            case XKB_KEY_g:
            case XKB_KEY_h:
            case XKB_KEY_i:
            case XKB_KEY_j:
            case XKB_KEY_k:
            case XKB_KEY_l:
            case XKB_KEY_m:
            case XKB_KEY_n:
            case XKB_KEY_o:
            case XKB_KEY_p:
            case XKB_KEY_q:
            case XKB_KEY_r:
            case XKB_KEY_s:
            case XKB_KEY_t:
            case XKB_KEY_u:
            case XKB_KEY_v:
            case XKB_KEY_w:
            case XKB_KEY_x:
            case XKB_KEY_y:
            case XKB_KEY_z:
                return static_cast<Input::Keycode>(Input::A + xkb_sym - XKB_KEY_a);

            case XKB_KEY_Super_L:
                return Input::LSUP;
            case XKB_KEY_Super_R:
                return Input::RSUP;
            // @todo: numpad?
            case XKB_KEY_multiply:  // This, and the below are not numpad, but in practice, makes sense to return
                                    // equivalent for numpad
                return Input::MULTIPLY;
            case XKB_KEY_decimalpoint:
                return Input::DEC;
            case XKB_KEY_F1:
            case XKB_KEY_F2:
            case XKB_KEY_F3:
            case XKB_KEY_F4:
            case XKB_KEY_F5:
            case XKB_KEY_F6:
            case XKB_KEY_F7:
            case XKB_KEY_F8:
            case XKB_KEY_F9:
            case XKB_KEY_F10:
            case XKB_KEY_F11:
            case XKB_KEY_F12:
            case XKB_KEY_F13:
            case XKB_KEY_F14:
            case XKB_KEY_F15:
            case XKB_KEY_F16:
            case XKB_KEY_F17:
            case XKB_KEY_F18:
            case XKB_KEY_F19:
            case XKB_KEY_F20:
            case XKB_KEY_F21:
            case XKB_KEY_F22:
            case XKB_KEY_F23:
            case XKB_KEY_F24:
                return static_cast<Input::Keycode>(Input::F1 + (xkb_sym - XKB_KEY_F1));  // @todo: check
            case XKB_KEY_Num_Lock:
                return Input::NUMLOCK;
            case XKB_KEY_Scroll_Lock:
                return Input::SCROLL;  // @todo: check
            case XKB_KEY_XF86AudioRaiseVolume:
                return Input::VOLUME_UP;
            case XKB_KEY_XF86AudioLowerVolume:
                return Input::VOLUME_DOWN;
            case XKB_KEY_XF86AudioMute:
                return Input::VOLUME_MUTE;

            case XKB_KEY_semicolon:
                return Input::SEMICOLON;
            case XKB_KEY_plus:
                return Input::PLUS;
            case XKB_KEY_comma:
                return Input::COMMA;
            case XKB_KEY_minus:
                return Input::MINUS;
            case XKB_KEY_period:
                return Input::PERIOD;
            case XKB_KEY_braceleft:
                return Input::LBRACE;
            case XKB_KEY_backslash:
                return Input::BACKSLASH;
            case XKB_KEY_braceright:
                return Input::RBRACE;
            case XKB_KEY_apostrophe:
                return Input::APOSTROPHE;
        }
        return Input::MAX_KEYS;
    }

    static void key(void *data,
                    struct wl_keyboard *wl_keyboard,
                    uint32_t serial,
                    uint32_t time,
                    uint32_t key,
                    uint32_t state) {
        (void)wl_keyboard;
        (void)serial;
        (void)time;

        WindowState *window_state = static_cast<WindowState *>(data);
        InternalState *internal   = static_cast<InternalState *>(window_state->get_internal_state());

        const xkb_keycode_t keycode = key + 8;
        xkb_keysym_t sym            = xkb_state_key_get_one_sym(internal->xkb_state, keycode);
        // @todo: assert(sym != XKB_KEY_NoSymbol)

        const Input::Keycode input_keycode = xkb_to_input(sym);
        if (input_keycode == Input::MAX_KEYS) return;

        const bool pressed = state == WL_KEYBOARD_KEY_STATE_PRESSED;
        window_state->input.process_key(input_keycode, pressed);
    }

    static void modifiers(void *data,
                          struct wl_keyboard *wl_keyboard,
                          uint32_t serial,
                          uint32_t mods_depressed,
                          uint32_t mods_latched,
                          uint32_t mods_locked,
                          uint32_t group) {
        (void)data;
        (void)wl_keyboard;
        (void)serial;
        (void)mods_depressed;
        (void)mods_latched;
        (void)mods_locked;
        (void)group;
    }

    static void repeat_info(void *data, struct wl_keyboard *wl_keyboard, int32_t rate, int32_t delay) {
        (void)data;
        (void)wl_keyboard;
        (void)rate;
        (void)delay;
    }

    wl_keyboard_listener listener = wl_keyboard_listener{
        .keymap      = keymap,
        .enter       = enter,
        .leave       = leave,
        .key         = key,
        .modifiers   = modifiers,
        .repeat_info = repeat_info,
    };
} wl_keyboard_listener;

const static struct {
    static void capabilities(void *data, struct wl_seat *wl_seat, uint32_t capabilities) {
        (void)wl_seat;
        WindowState *state      = static_cast<WindowState *>(data);
        InternalState *internal = static_cast<InternalState *>(state->get_internal_state());

        bool has_pointer  = capabilities & WL_SEAT_CAPABILITY_POINTER;
        bool has_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

        if (has_pointer && internal->wl_pointer == nullptr) {
            internal->wl_pointer = wl_seat_get_pointer(internal->wl_seat);
            wl_pointer_add_listener(internal->wl_pointer, &wl_pointer_listener.listener, state);
        } else if (!has_pointer && internal->wl_pointer != nullptr) {
            wl_pointer_release(internal->wl_pointer);
            internal->wl_pointer = nullptr;
        }

        if (has_keyboard && internal->wl_keyboard == nullptr) {
            internal->wl_keyboard = wl_seat_get_keyboard(internal->wl_seat);
            wl_keyboard_add_listener(internal->wl_keyboard, &wl_keyboard_listener.listener, state);
        } else if (!has_keyboard && internal->wl_keyboard != nullptr) {
            wl_keyboard_release(internal->wl_keyboard);
            internal->wl_keyboard = nullptr;
        }
    }
    static void name(void *data, struct wl_seat *wl_seat, const char *name) {
        (void)data;
        (void)wl_seat;
        printf("Name: %s\n", name);
    }
    wl_seat_listener listener = wl_seat_listener{
        .capabilities = capabilities,
        .name         = name,
    };
} wl_seat_listener;

const static struct {
    static void global(void *data,
                       struct wl_registry *wl_registry,
                       uint32_t name,
                       const char *interface,
                       uint32_t version) {
        (void)wl_registry;
        WindowState *state      = static_cast<WindowState *>(data);
        InternalState *internal = static_cast<InternalState *>(state->get_internal_state());

        if (strcmp(interface, wl_compositor_interface.name) == 0) {
            const uint32_t min_ver  = std::min<uint32_t>(7, version);
            internal->wl_compositor = static_cast<wl_compositor *>(
                wl_registry_bind(internal->wl_registry, name, &wl_compositor_interface, min_ver));
        } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
            const uint32_t min_ver = std::min<uint32_t>(7, version);
            internal->xdg_wm_base  = static_cast<xdg_wm_base *>(
                wl_registry_bind(internal->wl_registry, name, &xdg_wm_base_interface, min_ver));
            xdg_wm_base_add_listener(internal->xdg_wm_base, &xdg_wm_base_listener.listener, state);
        } else if (strcmp(interface, wl_seat_interface.name) == 0) {
            const uint32_t min_ver = std::min<uint32_t>(11, version);
            internal->wl_seat =
                static_cast<wl_seat *>(wl_registry_bind(internal->wl_registry, name, &wl_seat_interface, min_ver));
            wl_seat_add_listener(internal->wl_seat, &wl_seat_listener.listener, state);
        }

        // @temp remove after vulkan buffers
        if (strcmp(interface, wl_shm_interface.name) == 0) {
            const uint32_t min_ver = std::min<uint32_t>(3, version);
            internal->wl_shm =
                static_cast<wl_shm *>(wl_registry_bind(internal->wl_registry, name, &wl_shm_interface, min_ver));
        }
    }

    static void global_remove(void *data, struct wl_registry *wl_registry, uint32_t name) {
        (void)data;
        (void)wl_registry;
        (void)name;
        return;
    }

    const wl_registry_listener listener = wl_registry_listener{
        .global        = global,
        .global_remove = global_remove,
    };
} wl_registry_listener;

namespace Pastel {

WindowState::WindowState() {
    running        = true;
    width          = 0;
    height         = 0;
    internal_state = new InternalState();
};

WindowState::~WindowState() {
    InternalState *internal = static_cast<InternalState *>(internal_state);
    wl_shm_destroy(internal->wl_shm);

    xdg_surface_destroy(internal->xdg_surface);
    xdg_toplevel_destroy(internal->xdg_toplevel);

    wl_surface_destroy(internal->wl_surface);
    // Obtained from registry global callback
    if (internal->wl_pointer != nullptr) wl_pointer_destroy(internal->wl_pointer);
    if (internal->wl_keyboard != nullptr) wl_keyboard_destroy(internal->wl_keyboard);
    xdg_wm_base_destroy(internal->xdg_wm_base);
    wl_compositor_destroy(internal->wl_compositor);
    wl_seat_destroy(internal->wl_seat);

    wl_registry_destroy(internal->wl_registry);
    wl_display_disconnect(internal->wl_display);
    delete internal;
}

void WindowState::update() {
    prev_time    = current_time;
    current_time = get_time();

    input.input_update();
}

bool WindowState::open_window(const WindowConfig config) {
    InternalState *internal = static_cast<InternalState *>(internal_state);

    internal->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    if (internal->xkb_context == nullptr) {
        // @todo: log error message
        return false;
    }

    internal->wl_display = wl_display_connect(nullptr);
    if (internal->wl_display == nullptr) {
        // @todo: log error message
        return false;
    }
    internal->wl_registry = wl_display_get_registry(internal->wl_display);
    wl_registry_add_listener(internal->wl_registry, &wl_registry_listener.listener, this);
    wl_display_roundtrip(internal->wl_display);
    internal->wl_surface = wl_compositor_create_surface(internal->wl_compositor);

    internal->xdg_surface  = xdg_wm_base_get_xdg_surface(internal->xdg_wm_base, internal->wl_surface);
    internal->xdg_toplevel = xdg_surface_get_toplevel(internal->xdg_surface);
    xdg_toplevel_set_title(internal->xdg_toplevel, config.application_name);
    xdg_toplevel_add_listener(internal->xdg_toplevel, &xdg_toplevel_listener.listener, this);

    // @temp
    // the code below allocates a shared memory buffer. This is temporary to get something to show on the screen.
    // Remove after vulkan renderer.
    const int stride = config.width * 4;
    const int size   = stride * config.height;

    int fd = syscall(SYS_memfd_create, "buf", 0);
    ftruncate(fd, size);
    uint8_t *pool_data = static_cast<uint8_t *>(mmap(nullptr, size * 2, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    (void)pool_data;

    wl_shm_pool *pool = wl_shm_create_pool(internal->wl_shm, fd, size);
    wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, config.width, config.height, stride, WL_SHM_FORMAT_XRGB8888);

    wl_surface_attach(internal->wl_surface, buffer, 0, 0);
    wl_surface_commit(internal->wl_surface);

    return true;
}

bool WindowState::pump_window() {
    const InternalState *internal = static_cast<InternalState *>(internal_state);
    wl_display *const &display    = internal->wl_display;

    // We communicate our intent to read events. -1 is returned and EAGAIN is set if the event queue is nonempty.
    while (wl_display_prepare_read(display) == -1) {
        wl_display_dispatch_pending(display);
    }

    while (wl_display_flush(display) == -1) {
        if (errno != EAGAIN) {
            wl_display_cancel_read(display);
            return true;
        }
    }

    pollfd pfd = pollfd{
        .fd      = wl_display_get_fd(display),
        .events  = POLLIN,
        .revents = 0,
    };

    if (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN) != 0) {
        wl_display_read_events(display);
        wl_display_dispatch_pending(display);
    } else {
        wl_display_cancel_read(display);
    }

    return true;
}

constexpr int terminal_colour_to_code(Io::TerminalColour colour) {
    switch (colour) {
        case Io::TERMINAL_COLOUR_BLACK:
        case Io::TERMINAL_COLOUR_MAX:
            return 0;
        case Io::TERMINAL_COLOUR_RED:
            return 1;
        case Io::TERMINAL_COLOUR_GREEN:
            return 2;
        case Io::TERMINAL_COLOUR_YELLOW:
            return 3;
        case Io::TERMINAL_COLOUR_BLUE:
            return 4;
        case Io::TERMINAL_COLOUR_PURPLE:
            return 5;
        case Io::TERMINAL_COLOUR_LIGHTBLUE:
            return 6;
        case Io::TERMINAL_COLOUR_GRAY:
        case Io::TERMINAL_COLOUR_LIGHTGRAY:
        case Io::TERMINAL_COLOUR_WHITE:
            return 7;
        case Io::TERMINAL_COLOUR_NONE:
            return 9;
    }
    return 0;
}

void WindowState::print_terminal_raw(const char *msg) {
    int len = strlen(msg);
    ssize_t res;
    while (len > 0 && (res = write(STDOUT_FILENO, msg, len) != len)) {
        if (res < 0 && errno == EINTR) continue;
        if (res < 0) break;  // @todo: error

        len -= res;
        msg += res;
    }
}

void WindowState::clear_terminal_colour() {
    print_terminal_raw("\e[0m");
}

void WindowState::print_terminal(const char *msg, Io::TerminalColour fg, Io::TerminalColour bg) {
    const int fg_code = 30 + terminal_colour_to_code(fg);
    const int bg_code = 40 + terminal_colour_to_code(bg);

    char col_msg[64] = {};
    snprintf(col_msg, sizeof(col_msg), "\e[0;%d;%dm", fg_code, bg_code);
    print_terminal_raw(col_msg);
    print_terminal_raw(msg);
    clear_terminal_colour();
}

double WindowState::get_time() {
    timespec time_out;
    clock_gettime(CLOCK_MONOTONIC_RAW, &time_out);
    return time_out.tv_sec + time_out.tv_nsec * 0.000000001;
}

double WindowState::get_delta_time() {
    return current_time - prev_time;
}

void *WindowState::get_internal_state() {
    return internal_state;
}
}  // namespace Pastel

#endif
